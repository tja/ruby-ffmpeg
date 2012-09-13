#ifndef	RUBY_FFMPEG_VIDEO_FRAME_H
#define	RUBY_FFMPEG_VIDEO_FRAME_H

#include "ruby_ffmpeg.h"

// Object Lifetime
VALUE video_frame_register_class(VALUE module, VALUE super);
VALUE video_frame_new(AVFrame * frame, AVCodecContext * codec);
VALUE video_frame_new2(AVPicture * picture, int owner, int width, int height, int format, VALUE aspect, VALUE type, VALUE key, VALUE timestamp, VALUE duration);

#endif // RUBY_FFMPEG_VIDEO_FRAME_H
