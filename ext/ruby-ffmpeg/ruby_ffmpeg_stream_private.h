#ifndef	RUBY_FFMPEG_STREAM_PRIVATE_H
#define	RUBY_FFMPEG_STREAM_PRIVATE_H

#include "ruby_ffmpeg.h"

// Internal Data
typedef struct {
	AVStream *			stream;					// FFMPEG: Actual stream

	VALUE				reader;					// Ruby: Pointer back to reader
	VALUE				metadata;				// Ruby: Array of metadata
} StreamInternal;

// Object Lifetime
VALUE stream_alloc(VALUE klass);
void stream_mark(void * opaque);
void stream_free(void * opaque);

// Properties
VALUE stream_reader(VALUE self);

VALUE stream_index(VALUE self);
VALUE stream_type(VALUE self);
VALUE stream_tag(VALUE self);
VALUE stream_start_time(VALUE self);
VALUE stream_duration(VALUE self);
VALUE stream_frame_count(VALUE self);
VALUE stream_bit_rate(VALUE self);

VALUE stream_metadata(VALUE self);

#endif // RUBY_FFMPEG_STREAM_PRIVATE_H
