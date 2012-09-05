#ifndef	RUBY_FFMPEG_FRAME_H
#define	RUBY_FFMPEG_FRAME_H

#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

// Object Lifetime
VALUE frame_register_class(VALUE module);
VALUE frame_new(AVFrame * frame);

#endif // RUBY_FFMPEG_FRAME_H
