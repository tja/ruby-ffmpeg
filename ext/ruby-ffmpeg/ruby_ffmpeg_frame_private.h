#ifndef	RUBY_FFMPEG_FRAME_PRIVATE_H
#define	RUBY_FFMPEG_FRAME_PRIVATE_H

#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

// Properties
VALUE frame_raw_data(VALUE self);
VALUE frame_timestamp(VALUE self);
VALUE frame_duration(VALUE self);
VALUE frame_format(VALUE self);

// Methods

#endif // RUBY_FFMPEG_FRAME_PRIVATE_H
