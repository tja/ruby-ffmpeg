#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

#include "ruby_ffmpeg_util.h"

/*
**	Conversion.
*/

// Convert FFMPEG dictionary to Ruby hash
VALUE av_dictionary_to_ruby_hash(AVDictionary * dict) {
	VALUE metadata = rb_hash_new();

	AVDictionaryEntry * temp = NULL;
	while (temp = av_dict_get(dict, "", temp, AV_DICT_IGNORE_SUFFIX)) {
		rb_hash_aset(metadata, ID2SYM(rb_intern(temp->key)), rb_str_new2(temp->value));
	}

	return metadata;
}

// Convert FFMPEG error to Ruby string description
VALUE av_error_to_ruby_string(int error) {
	char temp[1024];
	av_strerror(error, &temp[0], sizeof(temp));
	return rb_str_new2(temp);
}

// Convert FFMPEG PixelFormat to symbol
VALUE av_pixel_format_to_symbol(enum PixelFormat format) {
	switch (format) {
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

// Convert FFMPEG SampleFormat to symbol
VALUE av_sample_format_to_symbol(enum AVSampleFormat format) {
	switch (format) {
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

// Convert FFMPEG MediaType to symbol
VALUE av_media_type_to_symbol(enum AVMediaType type) {
	switch (type) {
		case AVMEDIA_TYPE_VIDEO:			return ID2SYM(rb_intern("video"));
		case AVMEDIA_TYPE_AUDIO:			return ID2SYM(rb_intern("audio"));
		case AVMEDIA_TYPE_DATA:				return ID2SYM(rb_intern("data"));
		case AVMEDIA_TYPE_SUBTITLE:			return ID2SYM(rb_intern("subtitle"));
		case AVMEDIA_TYPE_ATTACHMENT:		return ID2SYM(rb_intern("attachment"));
		default:							return Qnil;
	}
}
