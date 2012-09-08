#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/pixdesc.h>
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
	char const * name = av_get_pix_fmt_name(format);
	return name ? ID2SYM(rb_intern(name)) : Qnil;
}

// Convert FFMPEG SampleFormat to symbol
VALUE av_sample_format_to_symbol(enum AVSampleFormat format) {
	char const * name = av_get_sample_fmt_name(format);
	return name ? ID2SYM(rb_intern(name)) : Qnil;
}

// Convert FFMPEG MediaType to symbol
VALUE av_media_type_to_symbol(enum AVMediaType type) {
	char const * name = av_get_media_type_string(type);
	return name ? ID2SYM(rb_intern(name)) : Qnil;
}
