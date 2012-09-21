#ifndef	RUBY_FFMPEG_AUDIO_RESAMPLER_H
#define	RUBY_FFMPEG_AUDIO_RESAMPLER_H

#include "ruby_ffmpeg.h"

// Object Lifetime
VALUE audio_resampler_register_class(VALUE module, VALUE super);
VALUE audio_resampler_new(VALUE object, int argc, VALUE * argv);

#endif // RUBY_FFMPEG_AUDIO_RESAMPLER_H
