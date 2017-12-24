# - Try to find ffmpeg libraries (libavcodec, libavformat and libavutil)
# Once done this will define
#
# FFMPEG_FOUND - system has ffmpeg or libav
# FFMPEG_INCLUDE_DIR - the ffmpeg include directory
# FFMPEG_LIBRARIES - Link these to use ffmpeg
# FFMPEG_LIBAVCODEC
# FFMPEG_LIBAVFORMAT
# FFMPEG_LIBAVUTIL
#
# Copyright (c) 2008 Andreas Schneider <mail@cynapses.org>
# Modified for other libraries by Lasse Kärkkäinen <tronic>
# Modified for Hedgewars by Stepik777
#
# Redistribution and use is allowed according to the terms of the New
# BSD license.
#

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
find_package(PkgConfig QUIET)
if (PKG_CONFIG_FOUND)
pkg_check_modules(_FFMPEG_AVCODEC libavcodec QUIET)
pkg_check_modules(_FFMPEG_AVFORMAT libavformat QUIET)
pkg_check_modules(_FFMPEG_AVUTIL libavutil QUIET)
endif (PKG_CONFIG_FOUND)

find_path(FFMPEG_AVCODEC_INCLUDE_DIR
NAMES libavcodec/avcodec.h
PATHS ${_FFMPEG_AVCODEC_INCLUDE_DIRS} /usr/include /usr/local/include /opt/local/include /sw/include
PATH_SUFFIXES ffmpeg libav
)

find_library(FFMPEG_LIBAVCODEC
NAMES avcodec
PATHS ${_FFMPEG_AVCODEC_LIBRARY_DIRS} /usr/lib /usr/local/lib /opt/local/lib /sw/lib
)

find_library(FFMPEG_LIBAVFORMAT
NAMES avformat
PATHS ${_FFMPEG_AVFORMAT_LIBRARY_DIRS} /usr/lib /usr/local/lib /opt/local/lib /sw/lib
)

find_library(FFMPEG_LIBAVUTIL
NAMES avutil
PATHS ${_FFMPEG_AVUTIL_LIBRARY_DIRS} /usr/lib /usr/local/lib /opt/local/lib /sw/lib
)

if (FFMPEG_LIBAVCODEC AND FFMPEG_LIBAVFORMAT)
set(FFMPEG_FOUND TRUE)
endif()

set(FFMPEG_INCLUDE_DIR ${FFMPEG_AVCODEC_INCLUDE_DIR})

set(FFMPEG_LIBRARIES
${FFMPEG_LIBAVCODEC}
${FFMPEG_LIBAVFORMAT}
${FFMPEG_LIBAVUTIL}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFmpeg DEFAULT_MSG FFMPEG_LIBRARIES FFMPEG_INCLUDE_DIR)

if(FFMPEG_FOUND AND NOT TARGET ffmpeg::ffmpeg)
    add_library(ffmpeg INTERFACE)
    target_link_libraries(ffmpeg
        INTERFACE
            ${FFMPEG_LIBRARIES}
        )
    target_include_directories(ffmpeg SYSTEM
        INTERFACE
            "${FFMPEG_INCLUDE_DIR}"
        )
    add_library(ffmpeg::ffmpeg ALIAS ffmpeg)
endif()