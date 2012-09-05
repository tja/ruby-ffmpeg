#ifndef	RUBY_FFMPEG_FRAME_PRIVATE_H
#define	RUBY_FFMPEG_FRAME_PRIVATE_H

#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

// Internal Data
typedef struct {
	AVFrame *			frame;					// FMPEG: Frame used for video decoding
} FrameInternal;


// Object Lifetime
VALUE frame_alloc(VALUE klass);
void frame_mark(void * opaque);
void frame_free(void * opaque);

// Properties
VALUE frame_width(VALUE self);
VALUE frame_height(VALUE self);

// Methods

#endif // RUBY_FFMPEG_FRAME_PRIVATE_H
