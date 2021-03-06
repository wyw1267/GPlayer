/*
 * Created by Gibbs on 2021/1/1.
 * Copyright (c) 2021 Gibbs. All rights reserved.
 */

#include <cstdlib>
#include "YuvGlesProgram.h"

#define TAG "YuvGlesProgram"

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI(TAG, "GL %s = %s\n", name, v);
}

static void checkGlError(const char *op) {
    for (GLint error = glGetError(); error; error = glGetError()) {
        LOGE(TAG, "after %s() glError (0x%x)\n", op, error);
    }
}

YuvGlesProgram::YuvGlesProgram() {
    _textureI = GL_TEXTURE0;
    _textureII = GL_TEXTURE1;
    _textureIII = GL_TEXTURE2;
    _tIindex = 0;
    _tIIindex = 1;
    _tIIIindex = 2;

    SQUARE_VERTICES = new GLfloat[8]{-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};// fullscreen
    COORD_VERTICES = new GLfloat[8]{0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f};// whole-texture
    MVP_MATRIX = new GLfloat[16]{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
}

YuvGlesProgram::~YuvGlesProgram() {
    delete[] SQUARE_VERTICES;
    delete[] COORD_VERTICES;
    delete[] MVP_MATRIX;
}

bool YuvGlesProgram::buildProgram() {
    checkGlError("before buildProgram");
    _program = createProgram(const_cast<char *>(VERTEX_SHADER),
                             const_cast<char *>(FRAGMENT_SHADER));
    LOGI(TAG, "_program = %d", _program);

    _mvpHandle = glGetUniformLocation(_program, "u_mvp");
    LOGI(TAG, "_mvpHandle = %d", _mvpHandle);
    checkGlError("glGetUniformLocation u_mvp");
    if (_mvpHandle == -1) {
        LOGE(TAG, "Could not get uniform location for u_mvp");
        return false;
    }

    /*
     * get handle for "a_position" and "a_texCoord"
     */
    _positionHandle = glGetAttribLocation(_program, "a_position");
    LOGI(TAG, "_positionHandle = %d", _positionHandle);
    checkGlError("glGetAttribLocation a_position");
    if (_positionHandle == -1) {
        LOGE(TAG, "Could not get attribute location for a_position");
        return false;
    }

    _coordHandle = glGetAttribLocation(_program, "a_texCoord");
    LOGI(TAG, "_coordHandle = %d", _coordHandle);
    checkGlError("glGetAttribLocation a_texCoord");
    if (_coordHandle == -1) {
        LOGE(TAG, "Could not get attribute location for a_texCoord");
        return false;
    }

    /*
     * get uniform location for y/u/v, we pass data through these uniforms
     */
    _yhandle = glGetUniformLocation(_program, "tex_y");
    LOGI(TAG, "_yhandle = %d", _yhandle);
    checkGlError("glGetUniformLocation tex_y");
    if (_yhandle == -1) {
        LOGE(TAG, "Could not get uniform location for tex_y");
        return false;
    }
    _uhandle = glGetUniformLocation(_program, "tex_u");
    LOGI(TAG, "_uhandle = %d", _uhandle);
    checkGlError("glGetUniformLocation tex_u");
    if (_uhandle == -1) {
        LOGE(TAG, "Could not get uniform location for tex_u");
        return false;
    }
    _vhandle = glGetUniformLocation(_program, "tex_v");
    LOGI(TAG, "_vhandle = %d", _vhandle);
    checkGlError("glGetUniformLocation tex_v");
    if (_vhandle == -1) {
        LOGE(TAG, "Could not get uniform location for tex_v");
        return false;
    }
    return true;
}

GLuint YuvGlesProgram::createProgram(char *vertexSource, char *fragmentSource) {
    LOGI(TAG, "createProgram vertexSource =\n%s", vertexSource);
    LOGI(TAG, "createProgram fragmentSource =\n%s", fragmentSource);
    // create shaders
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexSource);
    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource);
    // just check
    LOGI(TAG, "vertexShader = %d", vertexShader);
    LOGI(TAG, "pixelShader = %d", pixelShader);

    GLuint program = glCreateProgram();
    if (program != 0) {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
        glLinkProgram(program);
        checkGlError("glLinkProgram");
        GLint linkStatus = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char *buf = (char *) malloc(static_cast<size_t>(bufLength));
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, nullptr, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

GLuint YuvGlesProgram::loadShader(GLenum shaderType, char *source) {
    GLuint shader = glCreateShader(shaderType);
    if (shader != 0) {
        glShaderSource(shader, 1, &source, nullptr);
        checkGlError("glShaderSource");
        glCompileShader(shader);
        checkGlError("glCompileShader");
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (compiled != GL_TRUE) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char *buf = (char *) malloc(static_cast<size_t>(infoLen));
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, nullptr, buf);
                    LOGE("Could not compile shader :\n%s\n", buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    } else {
        checkGlError("glCreateShader");
    }
    return shader;
}

void YuvGlesProgram::buildTextures(uint8_t *y, uint8_t *u, uint8_t *v, uint32_t width, uint32_t height) {
    bool videoSizeChanged = (width != _video_width || height != _video_height);
    if (videoSizeChanged) {
        _video_width = width;
        _video_height = height;
        LOGI(TAG, "buildTextures videoSizeChanged: w=%d h=%d", _video_width, _video_height);
    }

    // building texture for Y data
    if (_ytid < 0 || videoSizeChanged) {
        if (_ytid >= 0) {
            LOGI(TAG, "glDeleteTextures Y");
            glDeleteTextures(1, &_ytid);
            checkGlError("glDeleteTextures");
        }
        // glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glGenTextures(1, &_ytid);
        checkGlError("glGenTextures");
        LOGI(TAG, "glGenTextures Y = %d", _ytid);
    }
    glBindTexture(GL_TEXTURE_2D, _ytid);
    checkGlError("glBindTexture");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, _video_width, _video_height, 0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE, y);
    checkGlError("glTexImage2D");
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // building texture for U data
    if (_utid < 0 || videoSizeChanged) {
        if (_utid >= 0) {
            LOGI(TAG, "glDeleteTextures U");
            glDeleteTextures(1, &_utid);
            checkGlError("glDeleteTextures");
        }
        glGenTextures(1, &_utid);
        checkGlError("glGenTextures");
        LOGI(TAG, "glGenTextures U = %d", _utid);
    }
    glBindTexture(GL_TEXTURE_2D, _utid);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, _video_width / 2, _video_height / 2, 0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE, u);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // building texture for V data
    if (_vtid < 0 || videoSizeChanged) {
        if (_vtid >= 0) {
            LOGI(TAG, "glDeleteTextures V");
            glDeleteTextures(1, &_vtid);
            checkGlError("glDeleteTextures");
        }
        glGenTextures(1, &_vtid);
        checkGlError("glGenTextures");
        LOGI(TAG, "glGenTextures V = %d", _vtid);
    }
    glBindTexture(GL_TEXTURE_2D, _vtid);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, _video_width / 2, _video_height / 2, 0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE, v);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void YuvGlesProgram::drawFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(_program);
    checkGlError("glUseProgram");

    glVertexAttribPointer(_positionHandle, 2, GL_FLOAT, GL_FALSE, 8, SQUARE_VERTICES);
    checkGlError("glVertexAttribPointer mPositionHandle");
    glEnableVertexAttribArray(_positionHandle);

    glVertexAttribPointer(_coordHandle, 2, GL_FLOAT, GL_FALSE, 8, COORD_VERTICES);
    checkGlError("glVertexAttribPointer maTextureHandle");
    glEnableVertexAttribArray(_coordHandle);

    glUniformMatrix4fv(_mvpHandle, 1, GL_FALSE, MVP_MATRIX);

    // bind textures
    glActiveTexture(_textureI);
    glBindTexture(GL_TEXTURE_2D, _ytid);
    glUniform1i(_yhandle, _tIindex);

    glActiveTexture(_textureII);
    glBindTexture(GL_TEXTURE_2D, _utid);
    glUniform1i(_uhandle, _tIIindex);

    glActiveTexture(_textureIII);
    glBindTexture(GL_TEXTURE_2D, _vtid);
    glUniform1i(_vhandle, _tIIIindex);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glFinish();

    glDisableVertexAttribArray(_positionHandle);
    glDisableVertexAttribArray(_coordHandle);
}
