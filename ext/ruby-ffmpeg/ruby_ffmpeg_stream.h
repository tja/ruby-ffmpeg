#ifndef	RUBY_FFMPEG_STREAM_H
#define	RUBY_FFMPEG_STREAM_H

#include <ruby.h>
#include <libavformat/avformat.h>

// Class object
extern VALUE stream_klass;

// entry points
VALUE stream_alloc(VALUE klass);

// properties
VALUE stream_format(VALUE self);
VALUE stream_index(VALUE self);
VALUE stream_type(VALUE self);
VALUE stream_tag(VALUE self);
VALUE stream_start_time(VALUE self);
VALUE stream_duration(VALUE self);
VALUE stream_frame_count(VALUE self);
VALUE stream_bit_rate(VALUE self);
VALUE stream_frame_rate(VALUE self);
VALUE stream_sample_rate(VALUE self);
VALUE stream_width(VALUE self);
VALUE stream_height(VALUE self);
VALUE stream_aspect_ratio(VALUE self);
VALUE stream_channels(VALUE self);
VALUE stream_metadata(VALUE self);

// helper
VALUE stream_create_instance(VALUE format, AVStream * stream);

#endif // RUBY_FFMPEG_STREAM_H