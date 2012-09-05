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
