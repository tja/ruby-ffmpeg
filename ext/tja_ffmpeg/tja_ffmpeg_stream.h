#ifndef	FFMPEG_STREAM_H
#define	FFMPEG_STREAM_H

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
VALUE stream_metadata(VALUE self);
VALUE stream_start_time(VALUE self);
VALUE stream_duration(VALUE self);
VALUE stream_frame_count(VALUE self);

// helper
VALUE stream_create_instance(VALUE format, AVStream * stream);

#endif // FFMPEG_STREAM_H