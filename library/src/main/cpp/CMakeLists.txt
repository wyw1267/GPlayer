# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

include_directories(
        .
        base
        codec
        codec/ffmpeg
        demuxing
        j4a
        media
        player
        render
        sender
        source)

# 定义源码所在目录
aux_source_directory(. SRC)
aux_source_directory(base SRC_BASE)
aux_source_directory(codec SRC_CODEC)
aux_source_directory(codec/ffmpeg SRC_CODEC_FFMPEG)
aux_source_directory(demuxing SRC_DEMUXING)
aux_source_directory(j4a SRC_J4A)
aux_source_directory(media SRC_MEDIA)
aux_source_directory(player SRC_PLAYER)
aux_source_directory(render SRC_RENDER)
aux_source_directory(source SRC_SOURCE)

list(APPEND SRC
        ${SRC_BASE}
        ${SRC_CODEC}
        ${SRC_CODEC_FFMPEG}
        ${SRC_DEMUXING}
        ${SRC_J4A}
        ${SRC_MEDIA}
        ${SRC_PLAYER}
        ${SRC_RENDER}
        ${SRC_SOURCE})

add_library( # Sets the name of the library.
        gplayer

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${SRC})

add_library(avcodec STATIC IMPORTED)
set_target_properties(avcodec
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/ffmpeg-lite/libavcodec.a)

add_library(avformat STATIC IMPORTED)
set_target_properties(avformat
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/ffmpeg-lite/libavformat.a)

add_library(avutil STATIC IMPORTED)
set_target_properties(avutil
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/ffmpeg-lite/libavutil.a)

add_library(swresample STATIC IMPORTED)
set_target_properties(swresample
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/ffmpeg-lite/libswresample.a)

add_library(swscale STATIC IMPORTED)
set_target_properties(swscale
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/ffmpeg-lite/libswscale.a)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        gplayer
        avformat
        avcodec
        avutil
        swresample
        swscale

        # Links the target library to the log library
        # included in the NDK.
        -landroid
        -lmediandk
        -lEGL
        -lGLESv2
        -lm -lz
        ${log-lib})