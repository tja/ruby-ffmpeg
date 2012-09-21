#ifndef	RUBY_FFMPEG_AUDIO_FRAME_PRIVATE_H
#define	RUBY_FFMPEG_AUDIO_FRAME_PRIVATE_H

#include "ruby_ffmpeg.h"

// Internal Data
typedef struct {
	uint8_t * 					samples;				// Raw sample data

	int							channels;				// FFMPEG: Audio channel count
	uint64_t					channel_layout;			// FFMPEG: Audio channel layout
	int							format;					// FFMPEG: Format of the picture data
	int							samples;				// FFMPEG: Sample count
	int							rate;					// FFMPEG: Sample rate

	VALUE						timestamp;				// Ruby: timestamp for this image (in seconds), or Qnil if not available
	VALUE						duration;				// Ruby: duration of this image (in seconds), or Qnil if not available
} AudioFrameInternal;

// Object Lifetime
VALUE audio_frame_alloc(VALUE klass);
void audio_frame_mark(void * opaque);
void audio_frame_free(void * opaque);

// Properties
VALUE audio_frame_data(VALUE self);

VALUE audio_frame_timestamp(VALUE self);
VALUE audio_frame_duration(VALUE self);
VALUE audio_frame_format(VALUE self);

VALUE audio_frame_channels(VALUE self);
VALUE audio_frame_channel_layout(VALUE self);
VALUE audio_frame_samples(VALUE self);
VALUE audio_frame_rate(VALUE self);

// Methods

#endif // RUBY_FFMPEG_AUDIO_FRAME_PRIVATE_H
