#include "ruby_ffmpeg.h"
#include "ruby_ffmpeg_util.h"


/*
**	Conversion.
*/

// Convert FFMPEG dictionary to Ruby hash
VALUE av_dictionary_to_ruby_hash(AVDictionary * dict) {
	VALUE metadata = rb_hash_new();

	AVDictionaryEntry * temp = NULL;
	while ((temp = av_dict_get(dict, "", temp, AV_DICT_IGNORE_SUFFIX)) != NULL) {
		rb_hash_aset(metadata, ID2SYM(rb_intern(temp->key)), rb_str_new2(temp->value));
	}

	return metadata;
}

// Convert FFMPEG PixelFormat to symbol
VALUE av_pixel_format_to_symbol(enum PixelFormat format) {
	char const * name = av_get_pix_fmt_name(format);
	return name ? ID2SYM(rb_intern(name)) : Qnil;
}

// Convert symbol to FFMPEG PixelFormat
enum PixelFormat symbol_to_av_pixel_format(VALUE symbol) {
	char const * name = rb_id2name(SYM2ID(symbol));
	return av_get_pix_fmt(name);
}

// Convert FFMPEG SampleFormat to symbol
VALUE av_sample_format_to_symbol(enum AVSampleFormat format) {
	char const * name = av_get_sample_fmt_name(format);
	return name ? ID2SYM(rb_intern(name)) : Qnil;
}

// Convert symbol to FFMPEG SampleFormat
enum AVSampleFormat symbol_to_av_sample_format(VALUE symbol) {
	char const * name = rb_id2name(SYM2ID(symbol));
	return av_get_sample_fmt(name);
}

// Convert FFMPEG MediaType to symbol
VALUE av_media_type_to_symbol(enum AVMediaType type) {
	switch (type) {
	case AVMEDIA_TYPE_VIDEO:		return ID2SYM(rb_intern("video"));
	case AVMEDIA_TYPE_AUDIO:		return ID2SYM(rb_intern("audio"));
	case AVMEDIA_TYPE_DATA:			return ID2SYM(rb_intern("data"));
	case AVMEDIA_TYPE_SUBTITLE:		return ID2SYM(rb_intern("subtitle"));
	case AVMEDIA_TYPE_ATTACHMENT:	return ID2SYM(rb_intern("attachment"));
	default:						return Qnil;
	}
}


/*
**	Misc.
*/

// Raise Ruby exception with FFMPEG error
void rb_raise_av_error(VALUE exception, int error) {
	static char temp[1024];
	av_strerror(error, &temp[0], sizeof(temp));

	rb_raise(exception, "%s", temp);
}

// Create new instance with the given arguments (using "first" in first place)
VALUE rb_class_new_instance2(VALUE first, int argc, VALUE * argv, VALUE klass) {
	// Create new argument array
	VALUE * new_argv = (VALUE *)av_mallocz((argc + 1) * sizeof(VALUE));
	if (!new_argv) rb_raise(rb_eNoMemError, "Failed to allocate memory");

	new_argv[0] = first;
	memcpy(&new_argv[1], argv, argc * sizeof(VALUE));

	// Create instance
	VALUE val = rb_class_new_instance(argc + 1, new_argv, klass);

	// Clean up
	av_free(new_argv);

	return val;
}
