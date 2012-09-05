#ifndef	RUBY_FFMPEG_UTIL_H
#define	RUBY_FFMPEG_UTIL_H

#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

// Conversion
VALUE av_dictionary_to_ruby_hash(AVDictionary * dict);
VALUE av_error_to_ruby_string(int error);

#endif // RUBY_FFMPEG_UTIL_H
