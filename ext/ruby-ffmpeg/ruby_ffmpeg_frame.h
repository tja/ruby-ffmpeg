#ifndef	RUBY_FFMPEG_FRAME_H
#define	RUBY_FFMPEG_FRAME_H

#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

// Object Lifetime
VALUE frame_register_class(VALUE module);

#endif // RUBY_FFMPEG_FRAME_H
