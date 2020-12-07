#include <media/MediaDataJni.h>
#include <base/Log.h>
#include "MediaSource.h"

#define TAG "MediaSourceC"

MediaSource::MediaSource() {
    LOGI(TAG, "CoreFlow : create MediaSource");
    mFormatInfo = static_cast<FormatInfo *>(malloc(sizeof(FormatInfo)));
    mFormatInfo->audcodecpar = avcodec_parameters_alloc();
    mFormatInfo->vidcodecpar = avcodec_parameters_alloc();
    mFormatInfo->subcodecpar = avcodec_parameters_alloc();
}

MediaSource::~MediaSource() {
    LOGI(TAG, "CoreFlow : MediaSource destroyed %d %d",
         audioPacketQueue.size(), videoPacketQueue.size());
    avcodec_parameters_free(&mFormatInfo->audcodecpar);
    avcodec_parameters_free(&mFormatInfo->vidcodecpar);
    avcodec_parameters_free(&mFormatInfo->subcodecpar);
    free(mFormatInfo);
}

void MediaSource::onInit(FormatInfo *formatInfo) {
    mFormatInfo = formatInfo;
}

uint32_t MediaSource::onReceiveAudio(MediaData *inPacket) {
    uint32_t queueSize = 0;
    audioPacketQueue.push_back(inPacket);
    queueSize = static_cast<uint32_t>(audioPacketQueue.size());
    return queueSize;
}

uint32_t MediaSource::onReceiveVideo(MediaData *inPacket) {
    uint32_t queueSize = 0;
    videoPacketQueue.push_back(inPacket);
    queueSize = static_cast<uint32_t>(videoPacketQueue.size());
    return queueSize;
}

void MediaSource::onRelease() {
}

FormatInfo *MediaSource::getFormatInfo() {
    return mFormatInfo;
}

AVCodecParameters *MediaSource::getAudioAVCodecParameters() {
    return mFormatInfo->audcodecpar;
}

AVCodecParameters *MediaSource::getVideoAVCodecParameters() {
    return mFormatInfo->vidcodecpar;
}

AVCodecParameters *MediaSource::getSubtitleAVCodecParameters() {
    return mFormatInfo->subcodecpar;
}

int MediaSource::readAudioBuffer(MediaData **avData) {
    if (audioPacketQueue.size() <= 0) {
        return AV_SOURCE_EMPTY;
    }
    *avData = audioPacketQueue.front();
    if (!(*avData)) {
        LOGE(TAG, "readAudioBuffer item is null");
        popAudioBuffer();
        return 0;
    }
    return static_cast<int>(audioPacketQueue.size());
}

int MediaSource::readVideoBuffer(MediaData **avData) {
    if (videoPacketQueue.size() <= 0) {
        return AV_SOURCE_EMPTY;
    }
    *avData = videoPacketQueue.front();
    if (!(*avData)) {
        LOGE(TAG, "readVideoBuffer item is null");
        popVideoBuffer();
        return 0;
    }
    return static_cast<int>(videoPacketQueue.size());
}

void MediaSource::popAudioBuffer() {
    mAudioLock.lock();
    if (audioPacketQueue.size() > 0) {
        audioPacketQueue.pop_front();
    }
    mAudioLock.unlock();
}

void MediaSource::popVideoBuffer() {
    mVideoLock.lock();
    if (videoPacketQueue.size() > 0) {
        videoPacketQueue.pop_front();
    }
    mVideoLock.unlock();
}

void MediaSource::flushBuffer() {
    LOGI(TAG, "CoreFlow : flushBuffer start");
    mVideoLock.lock();
    mAudioLock.lock();
    flushAudioBuffer();
    flushVideoBuffer();
    mAudioLock.unlock();
    mVideoLock.unlock();
    LOGI(TAG, "CoreFlow : flushBuffer end");
}

void MediaSource::flushVideoBuffer() {
    if (videoPacketQueue.size() > 0) {
        videoPacketQueue.clear();
    }
}

void MediaSource::flushAudioBuffer() {
    if (audioPacketQueue.size() > 0) {
        audioPacketQueue.clear();
    }
}

uint32_t MediaSource::getAudioBufferSize() {
    return static_cast<uint32_t>(audioPacketQueue.size());
}

uint32_t MediaSource::getVideoBufferSize() {
    return static_cast<uint32_t>(videoPacketQueue.size());
}
