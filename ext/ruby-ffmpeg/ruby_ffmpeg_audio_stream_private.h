#ifndef	RUBY_FFMPEG_AUDIO_STREAM_PRIVATE_H
#define	RUBY_FFMPEG_AUDIO_STREAM_PRIVATE_H

#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

#include "ruby_ffmpeg_stream_private.h"

// Internal Data
typedef struct {
	StreamInternal			base;					// Wrap base structure
} AudioStreamInternal;

// Object Lifetime
VALUE audio_stream_alloc(VALUE klass);
void audio_stream_mark(void * opaque);
void audio_stream_free(void * opaque);

// Properties
VALUE audio_stream_type(VALUE self);
VALUE audio_stream_format(VALUE self);

VALUE audio_stream_channels(VALUE self);
VALUE audio_stream_channel_layout(VALUE self);
VALUE audio_stream_sample_rate(VALUE self);

// Methods
VALUE audio_stream_decode(VALUE self);

#endif // RUBY_FFMPEG_AUDIO_STREAM_PRIVATE_H
