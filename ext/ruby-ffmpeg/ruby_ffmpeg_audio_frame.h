#ifndef	RUBY_FFMPEG_AUDIO_FRAME_H
#define	RUBY_FFMPEG_AUDIO_FRAME_H

#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

// Object Lifetime
VALUE audio_frame_register_class(VALUE module, VALUE super);
VALUE audio_frame_new(AVFrame * frame, AVCodecContext * codec);

#endif // RUBY_FFMPEG_AUDIO_FRAME_H
