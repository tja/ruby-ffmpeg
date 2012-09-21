#ifndef	RUBY_FFMPEG_VIDEO_STREAM_PRIVATE_H
#define	RUBY_FFMPEG_VIDEO_STREAM_PRIVATE_H

#include "ruby_ffmpeg.h"
#include "ruby_ffmpeg_stream_private.h"

// Internal Data
typedef struct {
	StreamInternal			base;					// Wrap base structure
} VideoStreamInternal;

// Object Lifetime
VALUE video_stream_alloc(VALUE klass);
void video_stream_mark(void * opaque);
void video_stream_free(void * opaque);

// Properties
VALUE video_stream_type(VALUE self);
VALUE video_stream_format(VALUE self);

VALUE video_stream_width(VALUE self);
VALUE video_stream_height(VALUE self);
VALUE video_stream_aspect_ratio(VALUE self);
VALUE video_stream_frame_rate(VALUE self);

// Methods
VALUE video_stream_resampler(int argc, VALUE * argv, VALUE self);
VALUE video_stream_decode(VALUE self);

#endif // RUBY_FFMPEG_VIDEO_STREAM_PRIVATE_H
