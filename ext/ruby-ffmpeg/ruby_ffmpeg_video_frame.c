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
	rb_define_method(_klass, "key?",				video_frame_key, 0);

	rb_define_method(_klass, "resample",			video_frame_resample, -1);

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
		if (internal->picture) {
			if (internal->owner)
				avpicture_free(internal->picture);
			av_free(internal->picture);
		}
		if (internal->scaler)
			sws_freeContext(internal->scaler);
		av_free(internal);
	}
}

// Mark for garbage collection
void video_frame_mark(void * opaque) {
	VideoFrameInternal * internal = (VideoFrameInternal *)opaque;
	if (internal) {
		rb_gc_mark(internal->aspect_ratio);
		rb_gc_mark(internal->picture_type);
		rb_gc_mark(internal->key);

		rb_gc_mark(internal->timestamp);
		rb_gc_mark(internal->duration);
	}
}

// Create new instance for given FFMPEG frame
VALUE video_frame_new(AVFrame * frame, AVCodecContext * codec) {
	// Time stamp: start of with best effort
	int64_t timestamp = frame->best_effort_timestamp;
	if (timestamp == AV_NOPTS_VALUE) {
		// Fall back to presentation timestamp of frame
		timestamp = frame->pts;
		if (timestamp == AV_NOPTS_VALUE) {
			// Fall back to presentation timestamp of packet
			timestamp = frame->pkt_pts;
			if (timestamp == AV_NOPTS_VALUE) {
				// Fall back to decompression timestamp of packet
				timestamp = frame->pkt_dts;
			}
		}
	}

	// Picture type
	VALUE picture_type = Qnil;

	switch (frame->pict_type) {
		case AV_PICTURE_TYPE_I:		picture_type = ID2SYM(rb_intern("i"));
		case AV_PICTURE_TYPE_P:		picture_type = ID2SYM(rb_intern("p"));
		case AV_PICTURE_TYPE_B:		picture_type = ID2SYM(rb_intern("b"));
		case AV_PICTURE_TYPE_S:		picture_type = ID2SYM(rb_intern("s"));
		case AV_PICTURE_TYPE_SI:	picture_type = ID2SYM(rb_intern("si"));
		case AV_PICTURE_TYPE_SP:	picture_type = ID2SYM(rb_intern("sp"));
		case AV_PICTURE_TYPE_BI:	picture_type = ID2SYM(rb_intern("bi"));
	}

	// Call main init method
	return video_frame_new2((AVPicture *)frame,
							0,
							frame->width,
							frame->height,
							frame->format,
							frame->sample_aspect_ratio.den ? rb_float_new(av_q2d(frame->sample_aspect_ratio)) : Qnil,
							picture_type,
							frame->key_frame ? Qtrue : Qfalse,
							(timestamp != AV_NOPTS_VALUE) ? rb_float_new(timestamp * av_q2d(codec->time_base)) : Qnil,
							(frame->pkt_duration != AV_NOPTS_VALUE) ? rb_float_new(frame->pkt_duration * av_q2d(codec->time_base)) : Qnil);
}

// Create new instance for given FFMPEG frame
VALUE video_frame_new2(AVPicture * picture, int owner, int width, int height, int format, VALUE aspect, VALUE type, VALUE key, VALUE timestamp, VALUE duration) {
	VALUE self = rb_class_new_instance(0, NULL, _klass);

	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	internal->picture = picture;
	internal->owner = owner;

	internal->width = width;
	internal->height = height;
	internal->format = format;
	internal->aspect_ratio = aspect;
	internal->picture_type = type;
	internal->key = key;

	internal->timestamp = timestamp;
	internal->duration = duration;

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
	int size = av_image_get_buffer_size(internal->format,
										internal->width,
										internal->height,
										1);
	if (size < 0) return Qnil;

	uint8_t * buffer = (uint8_t *)av_malloc(size);
	if (!buffer) rb_raise(rb_eNoMemError, "Failed to allocate image buffer");

	// Extract image data
	int err = av_image_copy_to_buffer(buffer,
									  size,
									  internal->picture->data,
									  internal->picture->linesize,
									  internal->format,
									  internal->width,
									  internal->height,
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

	return internal->timestamp;
}

// Duration of this frame (in seconds), nil if not available
VALUE video_frame_duration(VALUE self) {
	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	return internal->duration;
}

// Format of the frame, nil if not available
VALUE video_frame_format(VALUE self) {
	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	return av_pixel_format_to_symbol(internal->format);
}

// Video frame width (in pixels)
VALUE video_frame_width(VALUE self) {
	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	return INT2NUM(internal->width);
}

// Video frame height (in pixels)
VALUE video_frame_height(VALUE self) {
	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	return INT2NUM(internal->height);
}

// Video pixel aspect ratio, nil if not available
VALUE video_frame_aspect_ratio(VALUE self) {
	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	return internal->aspect_ratio;
}

// Picture type of the frame, nil if unknown
VALUE video_frame_picture_type(VALUE self) {
	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	return internal->picture_type;
}

// Is this a key frame?
VALUE video_frame_key(VALUE self) {
	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	return internal->key;
}


/*
**	Methods.
*/

// Rescale image
//
// frame.resample(factor)                        - Resize by percentage
// frame.resample(format)                        - Change color format
// frame.resample(width, height)                 - Resize to width and height
// frame.resample(width, height, filter)         - Resize to width and height using interpolation filter
// frame.resample(width, height, filter, format) - Resize to width and height using interpolation filter and change color format
VALUE video_frame_resample(int argc, VALUE * argv, VALUE self) {
	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	// Extract options
	int source_width 	= internal->width;
	int source_height	= internal->height;
	int source_format	= internal->format;
	int dest_width		= source_width;
	int dest_height		= source_height;
	int dest_format		= source_format;
	int flags			= SWS_FAST_BILINEAR;

	switch (argc) {
		case 0: {
			// Missing arguments
			rb_raise(rb_eArgError, "Missing argument(s)");
			break;
		}
		case 1: {
			if (TYPE(argv[0]) != T_SYMBOL) {
				// Resize by percentage
				dest_width	= source_width  * NUM2DBL(argv[0]);
				dest_height	= source_height * NUM2DBL(argv[0]);
			}
			else {
				// Change color format
				dest_format = symbol_to_av_pixel_format(argv[0]);
				if (dest_format == PIX_FMT_NONE) rb_raise(rb_eArgError, "Unknown color format");
			}
			break;
		}
		case 2: {
			// Resize to width and height
			dest_width = NUM2INT(argv[0]);
			dest_height = NUM2INT(argv[1]);
			break;
		}
		case 3: {
			// Resize to width and height using interpolation filter
			dest_width = NUM2INT(argv[0]);
			dest_height = NUM2INT(argv[1]);

			flags = symbol_to_interpolation_filter(argv[2]);
			if (flags == 0) rb_raise(rb_eArgError, "Unknown interpolation method");
			break;
		}
		case 4: {
			// Resize to width and height using interpolation filter and change color format
			dest_width = NUM2INT(argv[0]);
			dest_height = NUM2INT(argv[1]);

			flags = symbol_to_interpolation_filter(argv[2]);
			if (flags == 0) rb_raise(rb_eArgError, "Unknown interpolation method");

			dest_format = symbol_to_av_pixel_format(argv[3]);
			if (dest_format == PIX_FMT_NONE) rb_raise(rb_eArgError, "Unknown color format");
			break;
		}
		default: {
			// Too many arguments
			rb_raise(rb_eArgError, "Too many arguments");
			break;
		}
	}

	// Create or reuse scaler context
	internal->scaler = sws_getCachedContext(internal->scaler,
											source_width,
											source_height,
											source_format,
											dest_width,
											dest_height,
											dest_format,
											flags,
											NULL,
											NULL,
											NULL);

	// Create new picture structure
	AVPicture * dest_picture = (AVPicture *)av_mallocz(sizeof(AVPicture));
	if (!dest_picture) rb_raise(rb_eNoMemError, "Failed to allocate new picture");

	int err = avpicture_alloc(dest_picture, dest_format, dest_width, dest_height);
	if (err < 0) rb_raise(rb_eNoMemError, av_error_to_ruby_string(err));

	// Resample
	int height = sws_scale(internal->scaler,
						   internal->picture->data,
						   internal->picture->linesize,
						   0,
						   source_height,
						   dest_picture->data,
						   dest_picture->linesize);

	if (height != dest_height) rb_raise(rb_eRuntimeError, "Rescaling failed");

	// Wrap into Ruby object
	return video_frame_new2(dest_picture,
							1,
							dest_width,
							dest_height,
							dest_format,
							internal->aspect_ratio,
							internal->picture_type,
							internal->key,
							internal->timestamp,
							internal->duration);
}
