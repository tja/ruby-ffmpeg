#ifndef	RUBY_FFMPEG_VIDEO_RESAMPLER_PRIVATE_H
#define	RUBY_FFMPEG_VIDEO_RESAMPLER_PRIVATE_H

#include "ruby_ffmpeg.h"

// Internal Data
typedef struct {
	struct SwsContext *		context;				// FFMPEG: Video resampler context

	int 					src_width;				// Width of input image
	int 					src_height;				// Height of input image
	enum PixelFormat		src_format;				// Color format of input image

	int 					dst_width;				// Width of output image
	int 					dst_height;				// Height of output image
	enum PixelFormat		dst_format;				// Color format of output image

	int 					filter;					// Interpolation filter
} VideoResamplerInternal;

// Object Lifetime
VALUE video_resampler_alloc(VALUE klass);
void video_resampler_mark(void * opaque);
void video_resampler_free(void * opaque);

// Properties
VALUE video_resampler_src_width(VALUE self);
VALUE video_resampler_src_height(VALUE self);
VALUE video_resampler_src_format(VALUE self);
VALUE video_resampler_dst_width(VALUE self);
VALUE video_resampler_dst_height(VALUE self);
VALUE video_resampler_dst_format(VALUE self);
VALUE video_resampler_filter(VALUE self);

// Methods
VALUE video_resampler_initialize(int argc, VALUE * argv, VALUE self);
VALUE video_resampler_resample(VALUE self, VALUE frame);

// Helper Functions
VALUE interpolation_filter_to_symbol(int filter);
int symbol_to_interpolation_filter(VALUE symbol);

#endif // RUBY_FFMPEG_VIDEO_RESAMPLER_PRIVATE_H
