#ifndef	RUBY_FFMPEG_UTIL_H
#define	RUBY_FFMPEG_UTIL_H

#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

// Conversion
VALUE av_dictionary_to_ruby_hash(AVDictionary * dict);
VALUE av_error_to_ruby_string(int error);

VALUE av_pixel_format_to_symbol(enum PixelFormat format);
VALUE symbol_to_av_pixel_format(VALUE symbol);
VALUE av_sample_format_to_symbol(enum AVSampleFormat format);
VALUE av_media_type_to_symbol(enum AVMediaType type);

int symbol_to_interpolation_filter(VALUE filter);

#endif // RUBY_FFMPEG_UTIL_H
