#ifndef	FFMPEG_UTIL_H
#define	FFMPEG_UTIL_H

#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

VALUE av_dictionary_to_ruby_hash(AVDictionary * dict);

#endif // FFMPEG_UTIL_H