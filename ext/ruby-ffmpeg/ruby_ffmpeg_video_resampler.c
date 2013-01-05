#include "ruby_ffmpeg.h"
#include "ruby_ffmpeg_video_resampler.h"
#include "ruby_ffmpeg_video_resampler_private.h"
#include "ruby_ffmpeg_video_frame.h"
#include "ruby_ffmpeg_video_frame_private.h"
#include "ruby_ffmpeg_util.h"

// Globals
static VALUE _klass;


/*
**	Object Lifetime.
*/

// Register class
VALUE video_resampler_register_class(VALUE module, VALUE super) {
	_klass = rb_define_class_under(module, "VideoResampler", super);
	rb_define_alloc_func(_klass, video_resampler_alloc);

	rb_define_method(_klass, "src_width",	video_resampler_src_width, 0);
	rb_define_method(_klass, "src_height",	video_resampler_src_height, 0);
	rb_define_method(_klass, "src_format",	video_resampler_src_format, 0);
	rb_define_method(_klass, "dst_width",	video_resampler_dst_width, 0);
	rb_define_method(_klass, "dst_height",	video_resampler_dst_height, 0);
	rb_define_method(_klass, "dst_format",	video_resampler_dst_format, 0);
	rb_define_method(_klass, "filter",		video_resampler_filter, 0);

	rb_define_method(_klass, "initialize",	video_resampler_initialize, -1);
	rb_define_method(_klass, "resample",	video_resampler_resample, 1);
	rb_define_method(_klass, "^",			video_resampler_resample, 1);

	return _klass;
}

// Allocate object
VALUE video_resampler_alloc(VALUE klass) {
	VideoResamplerInternal * internal = (VideoResamplerInternal *)av_mallocz(sizeof(VideoResamplerInternal));
	if (!internal) rb_raise(rb_eNoMemError, "Failed to allocate internal structure");

	return Data_Wrap_Struct(klass, video_resampler_mark, video_resampler_free, (void *)internal);
}

// Free object
void video_resampler_free(void * opaque) {
	VideoResamplerInternal * internal = (VideoResamplerInternal *)opaque;
	if (internal) {
		if (internal->context)
			sws_freeContext(internal->context);
		av_free(internal);
	}
}

// Mark for garbage collection
void video_resampler_mark(void * opaque) {
	VideoResamplerInternal * internal = (VideoResamplerInternal *)opaque;
	if (internal) {
		// Nothing yet
	}
}

// Create new instance (from object)
VALUE video_resampler_new(VALUE object, int argc, VALUE * argv) {
	return rb_class_new_instance2(object, argc, argv, _klass);
}


/*
**	Properties.
*/

// Expected input width (in pixels)
VALUE video_resampler_src_width(VALUE self) {
	VideoResamplerInternal * internal;
	Data_Get_Struct(self, VideoResamplerInternal, internal);

	return INT2NUM(internal->src_width);
}

// Expected input height (in pixels)
VALUE video_resampler_src_height(VALUE self) {
	VideoResamplerInternal * internal;
	Data_Get_Struct(self, VideoResamplerInternal, internal);

	return INT2NUM(internal->src_height);
}

// Expected input format
VALUE video_resampler_src_format(VALUE self) {
	VideoResamplerInternal * internal;
	Data_Get_Struct(self, VideoResamplerInternal, internal);

	return av_pixel_format_to_symbol(internal->src_format);
}

// Resulting output width (in pixels)
VALUE video_resampler_dst_width(VALUE self) {
	VideoResamplerInternal * internal;
	Data_Get_Struct(self, VideoResamplerInternal, internal);

	return INT2NUM(internal->dst_width);
}

// Resulting output height (in pixels)
VALUE video_resampler_dst_height(VALUE self) {
	VideoResamplerInternal * internal;
	Data_Get_Struct(self, VideoResamplerInternal, internal);

	return INT2NUM(internal->dst_height);
}

// Resulting output format
VALUE video_resampler_dst_format(VALUE self) {
	VideoResamplerInternal * internal;
	Data_Get_Struct(self, VideoResamplerInternal, internal);

	return av_pixel_format_to_symbol(internal->dst_format);
}

// Interpolation filter
VALUE video_resampler_filter(VALUE self) {
	VideoResamplerInternal * internal;
	Data_Get_Struct(self, VideoResamplerInternal, internal);

	return interpolation_filter_to_symbol(internal->filter);
}


/*
**	Methods.
*/

// Initialize resampler
//
// Generic
//     ::new(src_width, src_height, src_format, dst_factor)									- Resize by percentage
//     ::new(src_width, src_height, src_format, dst_format)									- Change color format
//     ::new(src_width, src_height, src_format, dst_width, dst_height)						- Resize to width and height
//     ::new(src_width, src_height, src_format, dst_width, dst_height, filter)				- Resize with interpolation filter
//     ::new(src_width, src_height, src_format, dst_width, dst_height, dst_format, filter)	- Resize with filter and change color format
//
// From Object
//     ::new(source, dst_factor)															- Resize by percentage
//     ::new(source, dst_format)															- Change color format
//     ::new(source, dst_width, dst_height)													- Resize to width and height
//     ::new(source, dst_width, dst_height, filter)											- Resize with interpolation filter
//     ::new(source, dst_width, dst_height, dst_format, filter)								- Resize with filter and change color format
VALUE video_resampler_initialize(int argc, VALUE * argv, VALUE self) {
	VideoResamplerInternal * internal;
	Data_Get_Struct(self, VideoResamplerInternal, internal);

	if (argc && TYPE(argv[0]) == T_FIXNUM) {
		// Called generic form
		if 		(argc < 4)	rb_raise(rb_eArgError, "Missing argument(s)");
		else if (argc > 7)	rb_raise(rb_eArgError, "Too many arguments");

		internal->src_width 	= NUM2INT(argv[0]);
		internal->src_height	= NUM2INT(argv[1]);
		internal->src_format	= symbol_to_av_pixel_format(argv[2]);

		argc -= 3;
		argv += 3;
	}
	else {
		// Called with object
		if 		(argc < 2)	rb_raise(rb_eArgError, "Missing argument(s)");
		else if (argc > 5)	rb_raise(rb_eArgError, "Too many arguments");

		internal->src_width 	= NUM2INT(rb_funcall(argv[0], rb_intern("width"), 0));
		internal->src_height	= NUM2INT(rb_funcall(argv[0], rb_intern("height"), 0));
		internal->src_format	= symbol_to_av_pixel_format(rb_funcall(argv[0], rb_intern("format"), 0));

		argc -= 1;
		argv += 1;
	}

	internal->dst_width		= internal->src_width;
	internal->dst_height	= internal->src_height;
	internal->dst_format	= internal->src_format;
	internal->filter		= SWS_FAST_BILINEAR;

	switch (argc) {
		case 1: {
			if (TYPE(argv[0]) != T_SYMBOL) {
				// Resize by percentage
				internal->dst_width	= (int)(internal->src_width  * NUM2DBL(argv[0]));
				internal->dst_height = (int)(internal->src_height * NUM2DBL(argv[0]));
			}
			else {
				// Change color format
				internal->dst_format = symbol_to_av_pixel_format(argv[0]);
			}
			break;
		}
		case 2: {
			// Resize to width and height
			internal->dst_width = NUM2INT(argv[0]);
			internal->dst_height = NUM2INT(argv[1]);
			break;
		}
		case 3: {
			// Resize to width and height using interpolation filter
			internal->dst_width = NUM2INT(argv[0]);
			internal->dst_height = NUM2INT(argv[1]);
			internal->filter = symbol_to_interpolation_filter(argv[2]);
			break;
		}
		case 4: {
			// Resize to width and height using interpolation filter and change color format
			internal->dst_width = NUM2INT(argv[0]);
			internal->dst_height = NUM2INT(argv[1]);
			internal->dst_format = symbol_to_av_pixel_format(argv[2]);
			internal->filter = symbol_to_interpolation_filter(argv[3]);
			break;
		}
	}

	if (internal->src_format == PIX_FMT_NONE) rb_raise(rb_eArgError, "Unknown input color format");
	if (internal->dst_format == PIX_FMT_NONE) rb_raise(rb_eArgError, "Unknown output color format");
	if (internal->filter == 0) rb_raise(rb_eArgError, "Unknown interpolation method");

	// Create scaler context
	internal->context = sws_getContext(internal->src_width,
									   internal->src_height,
									   internal->src_format,
									   internal->dst_width,
									   internal->dst_height,
									   internal->dst_format,
									   internal->filter,
									   NULL,
									   NULL,
									   NULL);
	if (!internal->context)
		rb_raise(rb_eRuntimeError, "Failed to create rescaling context");

	return self;
}

// Resample image
VALUE video_resampler_resample(VALUE self, VALUE frame) {
	VideoResamplerInternal * internal;
	Data_Get_Struct(self, VideoResamplerInternal, internal);

	VideoFrameInternal * internal_frame;
	Data_Get_Struct(frame, VideoFrameInternal, internal_frame);

	// Create new picture structure
	AVPicture * dst_picture = (AVPicture *)av_mallocz(sizeof(AVPicture));
	if (!dst_picture) rb_raise(rb_eNoMemError, "Failed to allocate new picture");

	int err = avpicture_alloc(dst_picture, internal->dst_format, internal->dst_width, internal->dst_height);
	if (err < 0) rb_raise_av_error(rb_eNoMemError, err);

	// Resample
	int height = sws_scale(internal->context,
						   (uint8_t const * const *)internal_frame->picture->data,
						   (int const *)internal_frame->picture->linesize,
						   0,
						   internal->src_height,
						   dst_picture->data,
						   dst_picture->linesize);

	if (height != internal->dst_height) rb_raise(rb_eRuntimeError, "Rescaling failed");

	// Wrap into Ruby object
	return video_frame_new2(dst_picture,
							1,
							internal->dst_width,
							internal->dst_height,
							internal->dst_format,
							internal_frame->aspect_ratio,
							internal_frame->picture_type,
							internal_frame->key,
							internal_frame->timestamp,
							internal_frame->duration);
}


/*
**	Helper Functions.
*/

// Convert interpolation method to symbol
VALUE interpolation_filter_to_symbol(int filter) {
	switch (filter) {
		case SWS_FAST_BILINEAR:		return ID2SYM(rb_intern("fast_bilinear"));
		case SWS_BILINEAR:			return ID2SYM(rb_intern("bilinear"));
		case SWS_BICUBIC:			return ID2SYM(rb_intern("bicubic"));
		case SWS_X:					return ID2SYM(rb_intern("x"));
		case SWS_POINT:				return ID2SYM(rb_intern("point"));
		case SWS_AREA:				return ID2SYM(rb_intern("area"));
		case SWS_BICUBLIN:			return ID2SYM(rb_intern("bicublin"));
		case SWS_GAUSS:				return ID2SYM(rb_intern("gauss"));
		case SWS_SINC:				return ID2SYM(rb_intern("sinc"));
		case SWS_LANCZOS:			return ID2SYM(rb_intern("laczos"));
		case SWS_SPLINE:			return ID2SYM(rb_intern("spline"));
		default:					return Qnil;
	}
}

// Convert symbol to interpolation method
int symbol_to_interpolation_filter(VALUE symbol) {
	char const * name = rb_id2name(SYM2ID(symbol));

	if (strcmp(name, "fast_bilinear") == 0) return SWS_FAST_BILINEAR;
	if (strcmp(name, "bilinear") == 0)		return SWS_BILINEAR;
	if (strcmp(name, "bicubic") == 0)		return SWS_BICUBIC;
	if (strcmp(name, "x") == 0)				return SWS_X;
	if (strcmp(name, "point") == 0)			return SWS_POINT;
	if (strcmp(name, "area") == 0)			return SWS_AREA;
	if (strcmp(name, "bicublin") == 0)		return SWS_BICUBLIN;
	if (strcmp(name, "gauss") == 0)			return SWS_GAUSS;
	if (strcmp(name, "sinc") == 0)			return SWS_SINC;
	if (strcmp(name, "lanczos") == 0)		return SWS_LANCZOS;
	if (strcmp(name, "spline") == 0)		return SWS_SPLINE;
	return 0;
}
