#ifndef	RUBY_FFMPEG_AUDIO_FRAME_PRIVATE_H
#define	RUBY_FFMPEG_AUDIO_FRAME_PRIVATE_H

#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

// Internal Data
typedef struct {
	AVFrame *			frame;					// FFMPEG: Frame used for video decoding
	AVRational			time_base;				// Time base for this frame
	int					channels;				// Audio channel count
	uint64_t			channel_layout;			// Audio channel layout
} AudioFrameInternal;


// Object Lifetime
VALUE audio_frame_alloc(VALUE klass);
void audio_frame_mark(void * opaque);
void audio_frame_free(void * opaque);

// Properties
VALUE audio_frame_raw_data(VALUE self);

VALUE audio_frame_timestamp(VALUE self);
VALUE audio_frame_duration(VALUE self);
VALUE audio_frame_format(VALUE self);

VALUE audio_frame_channels(VALUE self);
VALUE audio_frame_channel_layout(VALUE self);
VALUE audio_frame_samples(VALUE self);
VALUE audio_frame_sample_rate(VALUE self);

// Methods

#endif // RUBY_FFMPEG_AUDIO_FRAME_PRIVATE_H
