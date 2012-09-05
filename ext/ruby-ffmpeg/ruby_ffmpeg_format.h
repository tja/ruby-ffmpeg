#ifndef	RUBY_FFMPEG_FORMAT_H
#define	RUBY_FFMPEG_FORMAT_H

#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

// Object Lifetime
VALUE format_create(VALUE module);
VALUE format_alloc(VALUE klass);
VALUE format_initialize(VALUE self, VALUE stream);

// Properties
VALUE format_name(VALUE self);
VALUE format_description(VALUE self);
VALUE format_start_time(VALUE self);
VALUE format_duration(VALUE self);
VALUE format_streams(VALUE self);
VALUE format_bit_rate(VALUE self);
VALUE format_metadata(VALUE self);

#endif // RUBY_FFMPEG_FORMAT_H