#ifndef	RUBY_FFMPEG_AUDIO_RESAMPLER_PRIVATE_H
#define	RUBY_FFMPEG_AUDIO_RESAMPLER_PRIVATE_H

#include "ruby_ffmpeg.h"

// Internal Data
typedef struct {
	struct ReSampleContext *	context;			// FFMPEG: Audio resampler context

	int 						src_channels;		// Channels for input frame
	int 						src_rate;			// Sample rate for input frame
	enum AVSampleFormat			src_format;			// Sample format for input frame

	int 						dst_channels;		// Channels for output frame
	int 						dst_rate;			// Sample rate for output frame
	enum AVSampleFormat			dst_format;			// Sample format for output frame
} AudioResamplerInternal;

// Object Lifetime
VALUE audio_resampler_alloc(VALUE klass);
void audio_resampler_mark(void * opaque);
void audio_resampler_free(void * opaque);

// Properties
VALUE audio_resampler_src_channels(VALUE self);
VALUE audio_resampler_src_rate(VALUE self);
VALUE audio_resampler_src_format(VALUE self);
VALUE audio_resampler_dst_channels(VALUE self);
VALUE audio_resampler_dst_rate(VALUE self);
VALUE audio_resampler_dst_format(VALUE self);

// Methods
VALUE audio_resampler_initialize(int argc, VALUE * argv, VALUE self);
VALUE audio_resampler_resample(VALUE self, VALUE frame);

#endif // RUBY_FFMPEG_AUDIO_RESAMPLER_PRIVATE_H
