#include "ruby_ffmpeg.h"
#include "ruby_ffmpeg_video_frame.h"
#include "ruby_ffmpeg_video_frame_private.h"
#include "ruby_ffmpeg_video_resampler.h"
#include "ruby_ffmpeg_util.h"

#include "util-bmp-format.h"

// Globals
static VALUE _klass;


/*
**	Object Lifetime.
*/

// Register class
VALUE video_frame_register_class(VALUE module, VALUE super) {
	_klass = rb_define_class_under(module, "VideoFrame", super);
	rb_define_alloc_func(_klass, video_frame_alloc);

	rb_define_method(_klass, "data",			video_frame_data, -1);
	rb_define_method(_klass, "timestamp",		video_frame_timestamp, 0);
	rb_define_method(_klass, "duration",		video_frame_duration, 0);
	rb_define_method(_klass, "format",			video_frame_format, 0);

	rb_define_method(_klass, "width",			video_frame_width, 0);
	rb_define_method(_klass, "height",			video_frame_height, 0);
	rb_define_method(_klass, "aspect_ratio",	video_frame_aspect_ratio, 0);
	rb_define_method(_klass, "picture_type",	video_frame_picture_type, 0);
	rb_define_method(_klass, "key?",			video_frame_key, 0);

	rb_define_method(_klass, "resampler", 		video_frame_resampler, -1);
	rb_define_method(_klass, "resample",		video_frame_resample, 1);
	rb_define_method(_klass, "^",				video_frame_resample, 1);

	rb_define_method(_klass, "to_bmp",			video_frame_to_bmp, 0);

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
		if (internal->rgba_conversion_context) {
			sws_freeContext(internal->rgba_conversion_context);
		}
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
	if (timestamp == (int64_t)AV_NOPTS_VALUE) {
		// Fall back to presentation timestamp of frame
		timestamp = frame->pts;
		if (timestamp == (int64_t)AV_NOPTS_VALUE) {
			// Fall back to presentation timestamp of packet
			timestamp = frame->pkt_pts;
			if (timestamp == (int64_t)AV_NOPTS_VALUE) {
				// Fall back to decompression timestamp of packet
				timestamp = frame->pkt_dts;
			}
		}
	}

	// Picture type
	VALUE picture_type = Qnil;

	switch (frame->pict_type) {
		case AV_PICTURE_TYPE_I:		picture_type = ID2SYM(rb_intern("i"));		break;
		case AV_PICTURE_TYPE_P:		picture_type = ID2SYM(rb_intern("p"));		break;
		case AV_PICTURE_TYPE_B:		picture_type = ID2SYM(rb_intern("b"));		break;
		case AV_PICTURE_TYPE_S:		picture_type = ID2SYM(rb_intern("s"));		break;
		case AV_PICTURE_TYPE_SI:	picture_type = ID2SYM(rb_intern("si"));		break;
		case AV_PICTURE_TYPE_SP:	picture_type = ID2SYM(rb_intern("sp"));		break;
		case AV_PICTURE_TYPE_BI:	picture_type = ID2SYM(rb_intern("bi"));		break;
		default:																break;
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
							(timestamp != (int64_t)AV_NOPTS_VALUE) ? rb_float_new(timestamp * av_q2d(codec->time_base)) : Qnil,
							(frame->pkt_duration != (int64_t)AV_NOPTS_VALUE) ? rb_float_new(frame->pkt_duration * av_q2d(codec->time_base)) : Qnil);
}

// Create new instance
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
//
// #data            - Get raw data with alignment 1
// #data(alignment) - Get raw data with given alignment
VALUE video_frame_data(int argc, VALUE * argv, VALUE self) {
	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	// Extract alignment
	if (argc > 1) rb_raise(rb_eArgError, "Too many arguments");
	int alignment = (argc == 1) ? NUM2INT(argv[0]) : 1;

	// Allocate buffer
	int size = av_image_get_buffer_size(internal->format,
										internal->width,
										internal->height,
										alignment);
	if (size < 0) return Qnil;

	uint8_t * buffer = (uint8_t *)av_malloc(size);
	if (!buffer) rb_raise(rb_eNoMemError, "Failed to allocate image buffer");

	// Extract image data
	int err = av_image_copy_to_buffer(buffer,
									  size,
									  (uint8_t const * const *)internal->picture->data,
									  (int const *)internal->picture->linesize,
									  internal->format,
									  internal->width,
									  internal->height,
									  alignment);
	if (err < 0) {
		av_free(buffer);
		rb_raise_av_error(rb_eRuntimeError, err);
	}

	// Wrap in ruby
	VALUE data = rb_str_new((char const *)buffer, size);
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

// Create resampler for object
VALUE video_frame_resampler(int argc, VALUE * argv, VALUE self) {
	return video_resampler_new(self, argc, argv);
}

// Resample video frame with given resampler
VALUE video_frame_resample(VALUE self, VALUE resampler) {
	return rb_funcall(resampler, rb_intern("resample"), 1, self);
}

// Export the image to Windows Bitmap format
VALUE video_frame_to_bmp(VALUE self) {
	VideoFrameInternal * internal;
	Data_Get_Struct(self, VideoFrameInternal, internal);

	// Allocate buffer for header and raw image data
	VALUE bmp_string = rb_str_new(NULL, util_bmp_format_get_header_size() + internal->width * internal->height * 4);

	// Fill in header
	uint8_t * bmp_data = util_bmp_format_write_header((uint8_t *)RSTRING_PTR(bmp_string),
													  internal->width,
													  internal->height);

	// Reuse resample context for color conversion to RGBA
	internal->rgba_conversion_context = sws_getCachedContext(internal->rgba_conversion_context,
															 internal->width,
													   		 internal->height,
															 internal->format,
															 internal->width,
															 internal->height,
															 PIX_FMT_RGBA,
															 SWS_POINT,
															 NULL,
															 NULL,
															 NULL);

	if (!internal->rgba_conversion_context)
		rb_raise(rb_eRuntimeError, "Failed to create RGBA rescaling context");

	// Resample directly into string buffer
	uint8_t * dst_data[1] = { bmp_data };
	int const dst_linesize[1] = { internal->width * 4 };
	
	int resampled_height = sws_scale(internal->rgba_conversion_context,
									 (uint8_t const * const *)internal->picture->data,
									 (int const *)internal->picture->linesize,
									 0,
									 internal->height,
									 dst_data,
									 dst_linesize);

	if (resampled_height != internal->height)
		rb_raise(rb_eRuntimeError, "Color conversion failed");

	// Return create string
	return bmp_string;
}
