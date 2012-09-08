#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

#include "ruby_ffmpeg_frame.h"
#include "ruby_ffmpeg_frame_private.h"
#include "ruby_ffmpeg_util.h"

// Globals
static VALUE _klass;


/*
**	Object Lifetime.
*/

// Register class
VALUE frame_register_class(VALUE module) {
	_klass = rb_define_class_under(module, "Frame", rb_cObject);
	rb_define_alloc_func(_klass, frame_alloc);

	rb_define_method(_klass, "data",				frame_data, 0);

	rb_define_method(_klass, "timestamp",			frame_timestamp, 0);
	rb_define_method(_klass, "duration",			frame_duration, 0);
	rb_define_method(_klass, "format",				frame_format, 0);
	rb_define_method(_klass, "key?",				frame_key, 0);

	rb_define_method(_klass, "width",				frame_width, 0);
	rb_define_method(_klass, "height",				frame_height, 0);
	rb_define_method(_klass, "aspect_ratio",		frame_aspect_ratio, 0);
	rb_define_method(_klass, "picture_type",		frame_picture_type, 0);
	rb_define_method(_klass, "interlaced?",			frame_interlaced, 0);
	rb_define_method(_klass, "top_field_first?",	frame_top_field_first, 0);

	rb_define_method(_klass, "channels",			frame_channels, 0);
	rb_define_method(_klass, "channel_layout",		frame_channel_layout, 0);
	rb_define_method(_klass, "samples",				frame_samples, 0);
	rb_define_method(_klass, "sample_rate",			frame_sample_rate, 0);

	return _klass;
}

// Allocate object
VALUE frame_alloc(VALUE klass) {
	FrameInternal * internal = (FrameInternal *)av_mallocz(sizeof(FrameInternal));
	if (!internal) rb_raise(rb_eNoMemError, "Failed to allocate internal structure");

	return Data_Wrap_Struct(klass, frame_mark, frame_free, (void *)internal);
}

// Free object
void frame_free(void * opaque) {
	FrameInternal * internal = (FrameInternal *)opaque;
	if (internal) {
		if (internal->frame)
			av_free(internal->frame);
		av_free(internal);
	}
}

// Mark for garbage collection
void frame_mark(void * opaque) {
	FrameInternal * internal = (FrameInternal *)opaque;
	if (internal) {
		// Nothing right now
	}
}

// Create new instance for given FFMPEG frame
VALUE frame_new(AVFrame * frame, AVCodecContext * codec) {
	VALUE self = rb_class_new_instance(0, NULL, _klass);

	FrameInternal * internal;
	Data_Get_Struct(self, FrameInternal, internal);

	internal->frame = frame;

	internal->time_base = codec->time_base;
	internal->channels = codec->channels;
	internal->channel_layout = codec->channel_layout;

	return self;
}


/*
**	Properties.
*/

// Return the raw data (as a string)
VALUE frame_data(VALUE self) {
	FrameInternal * internal;
	Data_Get_Struct(self, FrameInternal, internal);

	// Allocate big enough buffer
	int size = av_image_get_buffer_size(internal->frame->format, internal->frame->width, internal->frame->height, 1);
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

// Best effort timestamp, nil if not available
VALUE frame_timestamp(VALUE self) {
	FrameInternal * internal;
	Data_Get_Struct(self, FrameInternal, internal);

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

// Duration (in seconds) of this frame, nil if not available
VALUE frame_duration(VALUE self) {
	FrameInternal * internal;
	Data_Get_Struct(self, FrameInternal, internal);

	return (internal->frame->pkt_duration != AV_NOPTS_VALUE) ? rb_float_new(internal->frame->pkt_duration * av_q2d(internal->time_base)) : Qnil;
}

// Format of the frame, nil if not available
VALUE frame_format(VALUE self) {
	FrameInternal * internal;
	Data_Get_Struct(self, FrameInternal, internal);

	if (!internal->channels) {
		// Video formats
		return av_pixel_format_to_symbol(internal->frame->format);
	}
	else {
		// Audio formats
		return av_sample_format_to_symbol(internal->frame->format);
	}
}

// Is this a key frame?
VALUE frame_key(VALUE self) {
	FrameInternal * internal;
	Data_Get_Struct(self, FrameInternal, internal);

	return internal->frame->key_frame ? Qtrue : Qfalse;
}

// Video frame width (in pixels), nil if not available
VALUE frame_width(VALUE self) {
	FrameInternal * internal;
	Data_Get_Struct(self, FrameInternal, internal);

	return internal->frame->width ? INT2NUM(internal->frame->width) : Qnil;
}

// Video frame height (in pixels), nil if not available
VALUE frame_height(VALUE self) {
	FrameInternal * internal;
	Data_Get_Struct(self, FrameInternal, internal);

	return internal->frame->height ? INT2NUM(internal->frame->height) : Qnil;
}

// Video pixel aspect ratio, nil if not available
VALUE frame_aspect_ratio(VALUE self) {
	FrameInternal * internal;
	Data_Get_Struct(self, FrameInternal, internal);

	return internal->frame->sample_aspect_ratio.num ? rb_float_new(av_q2d(internal->frame->sample_aspect_ratio)) : Qnil;
}

// Picture type of the frame, nil if not available
VALUE frame_picture_type(VALUE self) {
	FrameInternal * internal;
	Data_Get_Struct(self, FrameInternal, internal);

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
VALUE frame_interlaced(VALUE self) {
	FrameInternal * internal;
	Data_Get_Struct(self, FrameInternal, internal);

	return internal->frame->interlaced_frame ? Qtrue : Qfalse;
}

// If interlaced video, is top field displayed first?
VALUE frame_top_field_first(VALUE self) {
	FrameInternal * internal;
	Data_Get_Struct(self, FrameInternal, internal);

	return internal->frame->top_field_first ? Qtrue : Qfalse;
}

// Number of audio channels, nil if not available
VALUE frame_channels(VALUE self) {
	FrameInternal * internal;
	Data_Get_Struct(self, FrameInternal, internal);

	return internal->channels ? INT2NUM(internal->channels) : Qnil;
}

// Layout of the audio channels, nil if not available
VALUE frame_channel_layout(VALUE self) {
	FrameInternal * internal;
	Data_Get_Struct(self, FrameInternal, internal);

	if (!internal->channels || !internal->channel_layout)
		return Qnil;

	char temp[64];
	av_get_channel_layout_string(&temp[0], sizeof(temp), internal->channels, internal->channels);
	return rb_str_new2(temp);
}

// Number of audio samples in this frame, nil if not available
VALUE frame_samples(VALUE self) {
	FrameInternal * internal;
	Data_Get_Struct(self, FrameInternal, internal);

	return internal->frame->nb_samples ? INT2NUM(internal->frame->nb_samples) : Qnil;
}

// Audio sample rate (samples per second), nil if not available
VALUE frame_sample_rate(VALUE self) {
	FrameInternal * internal;
	Data_Get_Struct(self, FrameInternal, internal);

	return av_frame_get_sample_rate(internal->frame) ? INT2NUM(av_frame_get_sample_rate(internal->frame)) : Qnil;
}
