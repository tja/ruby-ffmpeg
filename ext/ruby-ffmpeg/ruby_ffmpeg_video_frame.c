#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

#include "ruby_ffmpeg_video_frame.h"
#include "ruby_ffmpeg_video_frame_private.h"
#include "ruby_ffmpeg_util.h"

// Globals
static VALUE _klass;


/*
**	Object Lifetime.
*/

// Register class
VALUE video_frame_register_class(VALUE module, VALUE super) {
	_klass = rb_define_class_under(module, "VideoFrame", super);
	rb_define_alloc_func(_klass, video_frame_alloc);

	rb_define_method(_klass, "raw_data",			video_frame_raw_data, 0);
	rb_define_method(_klass, "timestamp",			video_frame_timestamp, 0);
	rb_define_method(_klass, "duration",			video_frame_duration, 0);
	rb_define_method(_klass, "format",				video_frame_format, 0);

	rb_define_method(_klass, "width",				video_frame_width, 0);
	rb_define_method(_klass, "height",				video_frame_height, 0);
	rb_define_method(_klass, "aspect_ratio",		video_frame_aspect_ratio, 0);
	rb_define_method(_klass, "picture_type",		video_frame_picture_type, 0);
	rb_define_method(_klass, "interlaced?",			video_frame_interlaced, 0);
	rb_define_method(_klass, "top_field_first?",	video_frame_top_field_first, 0);
	rb_define_method(_klass, "key?",				video_frame_key, 0);

	rb_define_method(_klass, "rescale",				video_frame_rescale, -1);

	return _klass;
}

// Allocate object
VALUE video_frame_alloc(VALUE klass) {
	VideoFrameInternal * internal = (VideoFrameInternal *)av_mallocz(sizeof(VideoFrameInternal));
	if (!internal) rb_raise(rb_eNoMemError, "Failed to allocate internal structure");

	return Data_Wrap_Struct(klass, video_frame_mark, video_frame_free, (void *)internal);
}

// Free object
void video_frame_free(void * opaque) {
	VideoFrameInternal * internal = (VideoFrameInternal *)opaque;
	if (internal) {
		if (internal->frame)
			av_free(internal->frame);
		av_free(internal);
	}
}

// Mark for garbage collection
void video_frame_mark(void * opaque) {
	VideoFrameInternal * internal = (VideoFrameInternal *)opaque;
	if (internal) {
		// Nothing right now
	}
}

// Create new instance for given FFMPEG frame
VALUE video_frame_new(AVFrame * frame, AVCodecContext * codec) {
	VALUE self = rb_class_new_instance(0, NULL, _klass);

	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	internal->frame = frame;
	internal->time_base = codec->time_base;

	return self;
}


/*
**	Properties.
*/

// Return the raw data (as string)
VALUE video_frame_raw_data(VALUE self) {
	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	// Allocate buffer
	int size = av_image_get_buffer_size(internal->frame->format,
										internal->frame->width,
										internal->frame->height,
										1);
	if (size < 0) return Qnil;

	uint8_t * buffer = (uint8_t *)av_malloc(size);
	if (!buffer) rb_raise(rb_eNoMemError, "Failed to allocate image buffer");

	// Extract image data
	int err = av_image_copy_to_buffer(buffer,
									  size,
									  internal->frame->data,
									  internal->frame->linesize,
									  internal->frame->format,
									  internal->frame->width,
									  internal->frame->height,
									  1);
	if (err < 0) {
		av_free(buffer);
		rb_raise(rb_eRuntimeError, av_error_to_ruby_string(err));
	}

	// Wrap in ruby
	VALUE data = rb_str_new(buffer, size);
	av_free(buffer);

	return data;
}

// Best effort timestamp (in seconds), nil if not available
VALUE video_frame_timestamp(VALUE self) {
	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	// Start of with best effort
	int64_t timestamp = internal->frame->best_effort_timestamp;
	if (timestamp != AV_NOPTS_VALUE)
		return rb_float_new(timestamp * av_q2d(internal->time_base));

	// Fall back to presentation timestamp of frame
	timestamp = internal->frame->pts;
	if (timestamp != AV_NOPTS_VALUE)
		return rb_float_new(timestamp * av_q2d(internal->time_base));

	// Fall back to presentation timestamp of packet
	timestamp = internal->frame->pkt_pts;
	if (timestamp != AV_NOPTS_VALUE)
		return rb_float_new(timestamp * av_q2d(internal->time_base));

	// Fall back to decompression timestamp of packet
	timestamp = internal->frame->pkt_dts;
	if (timestamp != AV_NOPTS_VALUE)
		return rb_float_new(timestamp * av_q2d(internal->time_base));

	return Qnil;
}

// Duration of this frame (in seconds), nil if not available
VALUE video_frame_duration(VALUE self) {
	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	return (internal->frame->pkt_duration != AV_NOPTS_VALUE) ? rb_float_new(internal->frame->pkt_duration * av_q2d(internal->time_base)) : Qnil;
}

// Format of the frame, nil if not available
VALUE video_frame_format(VALUE self) {
	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	return av_pixel_format_to_symbol(internal->frame->format);
}

// Video frame width (in pixels)
VALUE video_frame_width(VALUE self) {
	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	return INT2NUM(internal->frame->width);
}

// Video frame height (in pixels)
VALUE video_frame_height(VALUE self) {
	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	return INT2NUM(internal->frame->height);
}

// Video pixel aspect ratio, nil if not available
VALUE video_frame_aspect_ratio(VALUE self) {
	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	return internal->frame->sample_aspect_ratio.den ? rb_float_new(av_q2d(internal->frame->sample_aspect_ratio)) : Qnil;
}

// Picture type of the frame, nil if unknown
VALUE video_frame_picture_type(VALUE self) {
	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	switch (internal->frame->pict_type) {
		case AV_PICTURE_TYPE_I:			return ID2SYM(rb_intern("i"));
		case AV_PICTURE_TYPE_P:			return ID2SYM(rb_intern("p"));
		case AV_PICTURE_TYPE_B:			return ID2SYM(rb_intern("b"));
		case AV_PICTURE_TYPE_S:			return ID2SYM(rb_intern("s"));
		case AV_PICTURE_TYPE_SI:		return ID2SYM(rb_intern("si"));
		case AV_PICTURE_TYPE_SP:		return ID2SYM(rb_intern("sp"));
		case AV_PICTURE_TYPE_BI:		return ID2SYM(rb_intern("bi"));
		default:						return Qnil;
	}
}

// Is frame part of interlaced video?
VALUE video_frame_interlaced(VALUE self) {
	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	return internal->frame->interlaced_frame ? Qtrue : Qfalse;
}

// If interlaced video, is top field displayed first?
VALUE video_frame_top_field_first(VALUE self) {
	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	return internal->frame->top_field_first ? Qtrue : Qfalse;
}

// Is this a key frame?
VALUE video_frame_key(VALUE self) {
	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	return internal->frame->key_frame ? Qtrue : Qfalse;
}

// Rescale image
VALUE video_frame_rescale(int argc, VALUE * argv, VALUE self) {
	return self;
}
