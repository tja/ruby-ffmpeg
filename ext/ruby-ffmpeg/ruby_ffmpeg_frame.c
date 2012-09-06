#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

#include "ruby_ffmpeg_frame.h"
#include "ruby_ffmpeg_frame_private.h"

// Globals
static VALUE _klass;


/*
**	Object Lifetime.
*/

// Register class
VALUE frame_register_class(VALUE module) {
	_klass = rb_define_class_under(module, "Frame", rb_cObject);
	rb_define_alloc_func(_klass, frame_alloc);

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

	if (internal->channels == 0) {
		// Video formats
		switch (internal->frame->format) {
			case PIX_FMT_YUV420P:				return ID2SYM(rb_intern("yuv420p"));
			case PIX_FMT_YUYV422:				return ID2SYM(rb_intern("yuyv422"));
			case PIX_FMT_RGB24:					return ID2SYM(rb_intern("rgb24"));
			case PIX_FMT_BGR24:					return ID2SYM(rb_intern("bgr24"));
			case PIX_FMT_YUV422P:				return ID2SYM(rb_intern("yuv422p"));
			case PIX_FMT_YUV444P:				return ID2SYM(rb_intern("yuv444p"));
			case PIX_FMT_YUV410P:				return ID2SYM(rb_intern("yuv410p"));
			case PIX_FMT_YUV411P:				return ID2SYM(rb_intern("yuv411p"));
			case PIX_FMT_GRAY8:					return ID2SYM(rb_intern("gray8"));
			case PIX_FMT_MONOWHITE:				return ID2SYM(rb_intern("monowhite"));
			case PIX_FMT_MONOBLACK:				return ID2SYM(rb_intern("monoblack"));
			case PIX_FMT_PAL8:					return ID2SYM(rb_intern("pal8"));
			case PIX_FMT_YUVJ420P:				return ID2SYM(rb_intern("yuvj420p"));
			case PIX_FMT_YUVJ422P:				return ID2SYM(rb_intern("yuvj422p"));
			case PIX_FMT_YUVJ444P:				return ID2SYM(rb_intern("yuvj444p"));
			case PIX_FMT_XVMC_MPEG2_MC:			return ID2SYM(rb_intern("xvmc_mpeg2_mc"));
			case PIX_FMT_XVMC_MPEG2_IDCT:		return ID2SYM(rb_intern("xvmc_mpeg2_idct"));
			case PIX_FMT_UYVY422:				return ID2SYM(rb_intern("uyvy422"));
			case PIX_FMT_UYYVYY411:				return ID2SYM(rb_intern("uyyvyy411"));
			case PIX_FMT_BGR8:					return ID2SYM(rb_intern("bgr8"));
			case PIX_FMT_BGR4:					return ID2SYM(rb_intern("bgr4"));
			case PIX_FMT_BGR4_BYTE:				return ID2SYM(rb_intern("bgr4_byte"));
			case PIX_FMT_RGB8:					return ID2SYM(rb_intern("rgb8"));
			case PIX_FMT_RGB4:					return ID2SYM(rb_intern("rgb4"));
			case PIX_FMT_RGB4_BYTE:				return ID2SYM(rb_intern("rgb4_byte"));
			case PIX_FMT_NV12:					return ID2SYM(rb_intern("nv12"));
			case PIX_FMT_NV21:					return ID2SYM(rb_intern("nv21"));
			case PIX_FMT_ARGB:					return ID2SYM(rb_intern("argb"));
			case PIX_FMT_RGBA:					return ID2SYM(rb_intern("rgba"));
			case PIX_FMT_ABGR:					return ID2SYM(rb_intern("abgr"));
			case PIX_FMT_BGRA:					return ID2SYM(rb_intern("bgra"));
			case PIX_FMT_GRAY16BE:				return ID2SYM(rb_intern("gray16be"));
			case PIX_FMT_GRAY16LE:				return ID2SYM(rb_intern("gray16le"));
			case PIX_FMT_YUV440P:				return ID2SYM(rb_intern("yuv440p"));
			case PIX_FMT_YUVJ440P:				return ID2SYM(rb_intern("yuvj440p"));
			case PIX_FMT_YUVA420P:				return ID2SYM(rb_intern("yuva420p"));
			case PIX_FMT_VDPAU_H264:			return ID2SYM(rb_intern("vdpau_h264"));
			case PIX_FMT_VDPAU_MPEG1:			return ID2SYM(rb_intern("vdpau_mpeg1"));
			case PIX_FMT_VDPAU_MPEG2:			return ID2SYM(rb_intern("vdpau_mpeg2"));
			case PIX_FMT_VDPAU_WMV3:			return ID2SYM(rb_intern("vdpau_wmv3"));
			case PIX_FMT_VDPAU_VC1:				return ID2SYM(rb_intern("vdpau_vc1"));
			case PIX_FMT_RGB48BE:				return ID2SYM(rb_intern("rgb48be"));
			case PIX_FMT_RGB48LE:				return ID2SYM(rb_intern("rgb48le"));
			case PIX_FMT_RGB565BE:				return ID2SYM(rb_intern("rgb565be"));
			case PIX_FMT_RGB565LE:				return ID2SYM(rb_intern("rgb565le"));
			case PIX_FMT_RGB555BE:				return ID2SYM(rb_intern("rgb555be"));
			case PIX_FMT_RGB555LE:				return ID2SYM(rb_intern("rgb555le"));
			case PIX_FMT_BGR565BE:				return ID2SYM(rb_intern("bgr565be"));
			case PIX_FMT_BGR565LE:				return ID2SYM(rb_intern("bgr565le"));
			case PIX_FMT_BGR555BE:				return ID2SYM(rb_intern("bgr555be"));
			case PIX_FMT_BGR555LE:				return ID2SYM(rb_intern("bgr555le"));
			case PIX_FMT_VAAPI_MOCO:			return ID2SYM(rb_intern("vaapi_moco"));
			case PIX_FMT_VAAPI_IDCT:			return ID2SYM(rb_intern("vaapi_idct"));
			case PIX_FMT_VAAPI_VLD:				return ID2SYM(rb_intern("vaapi_vld"));
			case PIX_FMT_YUV420P16LE:			return ID2SYM(rb_intern("yuv420p16le"));
			case PIX_FMT_YUV420P16BE:			return ID2SYM(rb_intern("yuv420p16be"));
			case PIX_FMT_YUV422P16LE:			return ID2SYM(rb_intern("yuv422p16le"));
			case PIX_FMT_YUV422P16BE:			return ID2SYM(rb_intern("yuv422p16be"));
			case PIX_FMT_YUV444P16LE:			return ID2SYM(rb_intern("yuv444p16le"));
			case PIX_FMT_YUV444P16BE:			return ID2SYM(rb_intern("yuv444p16be"));
			case PIX_FMT_VDPAU_MPEG4:			return ID2SYM(rb_intern("vdpau_mpeg4"));
			case PIX_FMT_DXVA2_VLD:				return ID2SYM(rb_intern("dxva2_vld"));
			case PIX_FMT_RGB444LE:				return ID2SYM(rb_intern("rgb444le"));
			case PIX_FMT_RGB444BE:				return ID2SYM(rb_intern("rgb444be"));
			case PIX_FMT_BGR444LE:				return ID2SYM(rb_intern("bgr444le"));
			case PIX_FMT_BGR444BE:				return ID2SYM(rb_intern("bgr444be"));
			case PIX_FMT_GRAY8A:				return ID2SYM(rb_intern("gray8a"));
			case PIX_FMT_BGR48BE:				return ID2SYM(rb_intern("bgr48be"));
			case PIX_FMT_BGR48LE:				return ID2SYM(rb_intern("bgr48le"));
			case PIX_FMT_YUV420P9BE:			return ID2SYM(rb_intern("yuv420p9be"));
			case PIX_FMT_YUV420P9LE:			return ID2SYM(rb_intern("yuv420p9le"));
			case PIX_FMT_YUV420P10BE:			return ID2SYM(rb_intern("yuv420p10be"));
			case PIX_FMT_YUV420P10LE:			return ID2SYM(rb_intern("yuv420p10le"));
			case PIX_FMT_YUV422P10BE:			return ID2SYM(rb_intern("yuv422p10be"));
			case PIX_FMT_YUV422P10LE:			return ID2SYM(rb_intern("yuv422p10le"));
			case PIX_FMT_YUV444P9BE:			return ID2SYM(rb_intern("yuv444p9be"));
			case PIX_FMT_YUV444P9LE:			return ID2SYM(rb_intern("yuv444p9le"));
			case PIX_FMT_YUV444P10BE:			return ID2SYM(rb_intern("yuv444p10be"));
			case PIX_FMT_YUV444P10LE:			return ID2SYM(rb_intern("yuv444p10le"));
			case PIX_FMT_YUV422P9BE:			return ID2SYM(rb_intern("yuv422p9be"));
			case PIX_FMT_YUV422P9LE:			return ID2SYM(rb_intern("yuv422p9le"));
			case PIX_FMT_VDA_VLD:				return ID2SYM(rb_intern("vda_vld"));
			case PIX_FMT_GBRP:					return ID2SYM(rb_intern("gbrp"));
			case PIX_FMT_GBRP9BE:				return ID2SYM(rb_intern("gbrp9be"));
			case PIX_FMT_GBRP9LE:				return ID2SYM(rb_intern("gbrp9le"));
			case PIX_FMT_GBRP10BE:				return ID2SYM(rb_intern("gbrp10be"));
			case PIX_FMT_GBRP10LE:				return ID2SYM(rb_intern("gbrp10le"));
			case PIX_FMT_GBRP16BE:				return ID2SYM(rb_intern("gbrp16be"));
			case PIX_FMT_GBRP16LE:				return ID2SYM(rb_intern("gbrp16le"));
			case PIX_FMT_RGBA64BE:				return ID2SYM(rb_intern("rgba64be"));
			case PIX_FMT_RGBA64LE:				return ID2SYM(rb_intern("rgba64le"));
			case PIX_FMT_BGRA64BE:				return ID2SYM(rb_intern("bgra64be"));
			case PIX_FMT_BGRA64LE:				return ID2SYM(rb_intern("bgra64le"));
			case PIX_FMT_0RGB:					return ID2SYM(rb_intern("0rgb"));
			case PIX_FMT_RGB0:					return ID2SYM(rb_intern("rgb0"));
			case PIX_FMT_0BGR:					return ID2SYM(rb_intern("0bgr"));
			case PIX_FMT_BGR0:					return ID2SYM(rb_intern("bgr0"));
			case PIX_FMT_YUVA444P:				return ID2SYM(rb_intern("yuva444p"));
			case PIX_FMT_YUVA422P:				return ID2SYM(rb_intern("yuva422p"));
			case PIX_FMT_YUV420P12BE:			return ID2SYM(rb_intern("yuv420p12be"));
			case PIX_FMT_YUV420P12LE:			return ID2SYM(rb_intern("yuv420p12le"));
			case PIX_FMT_YUV420P14BE:			return ID2SYM(rb_intern("yuv420p14be"));
			case PIX_FMT_YUV420P14LE:			return ID2SYM(rb_intern("yuv420p14le"));
			case PIX_FMT_YUV422P12BE:			return ID2SYM(rb_intern("yuv422p12be"));
			case PIX_FMT_YUV422P12LE:			return ID2SYM(rb_intern("yuv422p12le"));
			case PIX_FMT_YUV422P14BE:			return ID2SYM(rb_intern("yuv422p14be"));
			case PIX_FMT_YUV422P14LE:			return ID2SYM(rb_intern("yuv422p14le"));
			case PIX_FMT_YUV444P12BE:			return ID2SYM(rb_intern("yuv444p12be"));
			case PIX_FMT_YUV444P12LE:			return ID2SYM(rb_intern("yuv444p12le"));
			case PIX_FMT_YUV444P14BE:			return ID2SYM(rb_intern("yuv444p14be"));
			case PIX_FMT_YUV444P14LE:			return ID2SYM(rb_intern("yuv444p14le"));
			case PIX_FMT_GBRP12BE:				return ID2SYM(rb_intern("gbrp12be"));
			case PIX_FMT_GBRP12LE:				return ID2SYM(rb_intern("gbrp12le"));
			case PIX_FMT_GBRP14BE:				return ID2SYM(rb_intern("gbrp14be"));
			case PIX_FMT_GBRP14LE:				return ID2SYM(rb_intern("gbrp14le"));
			default:							return Qnil;
		}
	}
	else {
		// Audio formats
		switch (internal->frame->format) {
			case AV_SAMPLE_FMT_S16:				return ID2SYM(rb_intern("s16"));
			case AV_SAMPLE_FMT_S32:				return ID2SYM(rb_intern("s32"));
			case AV_SAMPLE_FMT_FLT:				return ID2SYM(rb_intern("flt"));
			case AV_SAMPLE_FMT_DBL:				return ID2SYM(rb_intern("dbl"));
			case AV_SAMPLE_FMT_U8P:				return ID2SYM(rb_intern("u8p"));
			case AV_SAMPLE_FMT_S16P:			return ID2SYM(rb_intern("s16p"));
			case AV_SAMPLE_FMT_S32P:			return ID2SYM(rb_intern("s32p"));
			case AV_SAMPLE_FMT_FLTP:			return ID2SYM(rb_intern("fltp"));
			case AV_SAMPLE_FMT_DBLP:			return ID2SYM(rb_intern("dplp"));
			default:							return Qnil;
		}
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

