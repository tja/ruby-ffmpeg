#ifndef	RUBY_FFMPEG_STREAM_PRIVATE_H
#define	RUBY_FFMPEG_STREAM_PRIVATE_H

#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

// Internal Data
typedef struct {
	AVStream *			stream;					// FMPEG: Actual stream

	VALUE				format;					// Ruby: Pointer back to format
	VALUE				metadata;				// Ruby: Array of metadata
} StreamInternal;


// Object Lifetime
VALUE stream_alloc(VALUE klass);
void stream_mark(void * opaque);
void stream_free(void * opaque);

// Properties
VALUE stream_format(VALUE self);

VALUE stream_index(VALUE self);
VALUE stream_type(VALUE self);
VALUE stream_tag(VALUE self);
VALUE stream_start_time(VALUE self);
VALUE stream_duration(VALUE self);
VALUE stream_frame_count(VALUE self);

VALUE stream_bit_rate(VALUE self);

VALUE stream_width(VALUE self);
VALUE stream_height(VALUE self);
VALUE stream_aspect_ratio(VALUE self);
VALUE stream_frame_rate(VALUE self);

VALUE stream_channels(VALUE self);
VALUE stream_sample_rate(VALUE self);

VALUE stream_metadata(VALUE self);

#endif // RUBY_FFMPEG_STREAM_PRIVATE_H
