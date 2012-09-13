#ifndef	RUBY_FFMPEG_AUDIO_FRAME_H
#define	RUBY_FFMPEG_AUDIO_FRAME_H

#include "ruby_ffmpeg.h"

// Object Lifetime
VALUE audio_frame_register_class(VALUE module, VALUE super);
VALUE audio_frame_new(AVFrame * frame, AVCodecContext * codec);
VALUE audio_frame_new2(uint8_t * samples, int channels, uint64_t channel_layout, enum AVSampleFormat format, int sample_count, int sample_rate, VALUE timestamp, VALUE duration);

#endif // RUBY_FFMPEG_AUDIO_FRAME_H
