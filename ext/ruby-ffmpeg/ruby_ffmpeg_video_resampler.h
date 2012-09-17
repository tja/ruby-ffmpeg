#ifndef	RUBY_FFMPEG_VIDEO_RESAMPLER_H
#define	RUBY_FFMPEG_VIDEO_RESAMPLER_H

#include "ruby_ffmpeg.h"

// Object Lifetime
VALUE video_resampler_register_class(VALUE module, VALUE super);
VALUE video_resampler_new(VALUE object, int argc, VALUE * argv);

#endif // RUBY_FFMPEG_VIDEO_RESAMPLER_H
