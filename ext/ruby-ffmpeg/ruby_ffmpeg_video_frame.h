#ifndef	RUBY_FFMPEG_VIDEO_FRAME_H
#define	RUBY_FFMPEG_VIDEO_FRAME_H

#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

// Object Lifetime
VALUE video_frame_register_class(VALUE module, VALUE super);
VALUE video_frame_new(AVFrame * frame, AVCodecContext * codec);

#endif // RUBY_FFMPEG_VIDEO_FRAME_H
