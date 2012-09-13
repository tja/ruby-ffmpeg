#ifndef	RUBY_FFMPEG_UTIL_H
#define	RUBY_FFMPEG_UTIL_H

#include "ruby_ffmpeg.h"

// Conversion
VALUE av_dictionary_to_ruby_hash(AVDictionary * dict);

VALUE av_pixel_format_to_symbol(enum PixelFormat format);
enum PixelFormat symbol_to_av_pixel_format(VALUE symbol);

VALUE av_sample_format_to_symbol(enum AVSampleFormat format);

VALUE av_media_type_to_symbol(enum AVMediaType type);

int symbol_to_interpolation_filter(VALUE filter);

// Misc
void rb_raise_av_error(VALUE exception, int error);


#endif // RUBY_FFMPEG_UTIL_H
