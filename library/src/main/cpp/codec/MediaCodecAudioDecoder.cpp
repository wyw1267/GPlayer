/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#include <cstring>
#include <base/Log.h>
#include "MediaCodecAudioDecoder.h"

extern "C" {
#include <demuxing/avformat_def.h>
}

#define TAG "MediaCodecAudioDecoder"

MediaCodecAudioDecoder::MediaCodecAudioDecoder() = default;

MediaCodecAudioDecoder::~MediaCodecAudioDecoder() = default;

void MediaCodecAudioDecoder::init(AVCodecParameters *codecParameters) {
    const char *mine = get_mime_by_codec_id(static_cast<CODEC_TYPE>(codecParameters->codec_id));
    mAMediaCodec = AMediaCodec_createDecoderByType(mine);
    if (!mAMediaCodec) {
        LOGE(TAG, "can not find mine %s", mine);
        return;
    }

    int sampleRate = codecParameters->sample_rate;
    int channelCount = codecParameters->channels;
    int profile = 2;
    LOGI(TAG, "CoreFlow : init mine=%s,sampleRate=%d,channelCount=%d", mine, sampleRate,
         channelCount);

    AMediaFormat *audioFormat = AMediaFormat_new();
    AMediaFormat_setString(audioFormat, AMEDIAFORMAT_KEY_MIME, mine);
    AMediaFormat_setInt32(audioFormat, AMEDIAFORMAT_KEY_SAMPLE_RATE, sampleRate);
    AMediaFormat_setInt32(audioFormat, AMEDIAFORMAT_KEY_CHANNEL_COUNT, channelCount);
    AMediaFormat_setInt32(audioFormat, AMEDIAFORMAT_KEY_AAC_PROFILE, profile);
    AMediaFormat_setInt32(audioFormat, AMEDIAFORMAT_KEY_IS_ADTS, 1);
    AMediaFormat_setBuffer(audioFormat, "csd-0", codecParameters->extradata,
                           codecParameters->extradata_size);

    media_status_t status = AMediaCodec_configure(mAMediaCodec, audioFormat, nullptr, nullptr, 0);
    if (status != AMEDIA_OK) {
        LOGE(TAG, "configure fail %d", status);
        AMediaCodec_delete(mAMediaCodec);
        mAMediaCodec = nullptr;
        return;
    }

    status = AMediaCodec_start(mAMediaCodec);
    if (status != AMEDIA_OK) {
        LOGE(TAG, "start fail %d", status);
        AMediaCodec_delete(mAMediaCodec);
        mAMediaCodec = nullptr;
        return;
    }
    LOGI(TAG, "start successfully");
}

int MediaCodecAudioDecoder::send_packet(AVPacket *inPacket) {
    if (!mAMediaCodec) {
        return INVALID_CODEC;
    }

    ssize_t bufferId = AMediaCodec_dequeueInputBuffer(mAMediaCodec, 2000);
    if (bufferId >= 0) {
        uint32_t flag = 0;
        if ((inPacket->flags & AV_PKT_FLAG_KEY) == AV_PKT_FLAG_KEY) {
            flag = AMEDIACODEC_BUFFER_FLAG_PARTIAL_FRAME;
        }
        // 获取buffer的索引
        size_t outsize;
        uint8_t *inputBuf = AMediaCodec_getInputBuffer(mAMediaCodec, bufferId, &outsize);
        if (inputBuf != nullptr && inPacket->size <= outsize) {
            // 将待解码的数据copy到硬件中
            memcpy(inputBuf, inPacket->data, inPacket->size);
            media_status_t status = AMediaCodec_queueInputBuffer(mAMediaCodec, bufferId, 0,
                                                                 inPacket->size, inPacket->pts,
                                                                 flag);
            return (status == AMEDIA_OK ? 0 : -2);
        }
    }
    return TRY_AGAIN;
}

int MediaCodecAudioDecoder::receive_frame(MediaData *outFrame) {
    if (!mAMediaCodec) {
        return INVALID_CODEC;
    }

    AMediaCodecBufferInfo info;
    ssize_t bufferId = AMediaCodec_dequeueOutputBuffer(mAMediaCodec, &info, 2000);
    if (bufferId >= 0) {
        size_t outsize;
        uint8_t *outputBuf = AMediaCodec_getOutputBuffer(mAMediaCodec, bufferId, &outsize);
        if (outputBuf != nullptr) {
            extractFrame(outputBuf, outFrame, info);
            AMediaCodec_releaseOutputBuffer(mAMediaCodec, bufferId, false);
            return 0;
        }
    } else if (bufferId == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
        auto format = AMediaCodec_getOutputFormat(mAMediaCodec);
        int32_t localColorFMT;
        AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_COLOR_FORMAT, &localColorFMT);
        return -2;
    } else if (bufferId == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {

    }
    return -3;
}

void MediaCodecAudioDecoder::release() {
    if (mAMediaCodec) {
        AMediaCodec_flush(mAMediaCodec);
        AMediaCodec_stop(mAMediaCodec);
        AMediaCodec_delete(mAMediaCodec);
        mAMediaCodec = nullptr;
    }
}

void MediaCodecAudioDecoder::reset() {
    AMediaCodec_flush(mAMediaCodec);
}

void MediaCodecAudioDecoder::extractFrame(uint8_t *outputBuf, MediaData *outFrame,
                                     AMediaCodecBufferInfo info) {
    outFrame->pts = info.presentationTimeUs;
    outFrame->dts = outFrame->pts;
    uint32_t frameSize = info.size;
    memcpy(outFrame->data, outputBuf, frameSize);
    outFrame->size = frameSize;
}
