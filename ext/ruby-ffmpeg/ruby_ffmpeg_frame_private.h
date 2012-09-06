#ifndef	RUBY_FFMPEG_FRAME_PRIVATE_H
#define	RUBY_FFMPEG_FRAME_PRIVATE_H

#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

// Internal Data
typedef struct {
	AVFrame *			frame;					// FMPEG: Frame used for video decoding
	AVCodecContext *	codec;					// FMPEG: Codec context used to decode this frame
} FrameInternal;


// Object Lifetime
VALUE frame_alloc(VALUE klass);
void frame_mark(void * opaque);
void frame_free(void * opaque);

// Properties
VALUE frame_timestamp(VALUE self);
VALUE frame_duration(VALUE self);
VALUE frame_key(VALUE self);

VALUE frame_width(VALUE self);
VALUE frame_height(VALUE self);
VALUE frame_aspect_ratio(VALUE self);
VALUE frame_interlaced(VALUE self);
VALUE frame_top_field_first(VALUE self);

VALUE frame_channels(VALUE self);
VALUE frame_channel_layout(VALUE self);
VALUE frame_samples(VALUE self);
VALUE frame_sample_rate(VALUE self);

// Methods

#endif // RUBY_FFMPEG_FRAME_PRIVATE_H
