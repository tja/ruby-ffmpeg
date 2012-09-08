#ifndef	RUBY_FFMPEG_VIDEO_FRAME_PRIVATE_H
#define	RUBY_FFMPEG_VIDEO_FRAME_PRIVATE_H

#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

// Internal Data
typedef struct {
	AVFrame *			frame;					// FFMPEG: Frame used for video decoding
	AVRational			time_base;				// Time base for this frame
} VideoFrameInternal;


// Object Lifetime
VALUE video_frame_alloc(VALUE klass);
void video_frame_mark(void * opaque);
void video_frame_free(void * opaque);

// Properties
VALUE video_frame_raw_data(VALUE self);
VALUE video_frame_timestamp(VALUE self);
VALUE video_frame_duration(VALUE self);
VALUE video_frame_format(VALUE self);

VALUE video_frame_width(VALUE self);
VALUE video_frame_height(VALUE self);
VALUE video_frame_aspect_ratio(VALUE self);
VALUE video_frame_picture_type(VALUE self);
VALUE video_frame_interlaced(VALUE self);
VALUE video_frame_top_field_first(VALUE self);
VALUE video_frame_key(VALUE self);

// Methods
VALUE video_frame_rescale(int argc, VALUE * argv, VALUE self);

#endif // VIDEO_
