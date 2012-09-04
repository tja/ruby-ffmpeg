#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

#include "tja_ffmpeg_util.h"

/*
**
*/
VALUE av_dictionary_to_ruby_hash(AVDictionary * dict) {
	VALUE metadata = rb_hash_new();

	AVDictionaryEntry * temp = NULL;
	while (temp = av_dict_get(dict, "", temp, AV_DICT_IGNORE_SUFFIX)) {
		rb_hash_aset(metadata, ID2SYM(rb_intern(temp->key)), rb_str_new2(temp->value));
	}

	return metadata;
}
