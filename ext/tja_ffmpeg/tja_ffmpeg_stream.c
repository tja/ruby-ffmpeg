#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

#include "tja_ffmpeg_stream.h"
#include "tja_ffmpeg_util.h"

// ...
VALUE stream_klass;

// Internal data
typedef struct {
	// LibAV
	AVStream *			stream;
	// Ruby
	VALUE				format;
	VALUE				metadata;
} Stream_Internal;

/*
**
*/
void stream_mark(void * opaque) {
	Stream_Internal * internal = (Stream_Internal *)opaque;
	if (internal) {
		rb_gc_mark(internal->format);
		rb_gc_mark(internal->metadata);
	}
}

/*
**
*/
void stream_free(void * opaque) {
	Stream_Internal * internal = (Stream_Internal *)opaque;
	if (internal) {
		// nothing yet
	}
}

/*
**
*/
VALUE stream_alloc(VALUE klass) {
	Stream_Internal * internal = (Stream_Internal *)av_mallocz(sizeof(Stream_Internal));
	if (!internal) rb_raise(rb_eNoMemError, "Filed to allocate internal structure");

	return Data_Wrap_Struct(klass, stream_mark, stream_free, (void *)internal);
}

/*
**
*/
VALUE stream_create_instance(VALUE format, AVStream * stream) {
	VALUE self = rb_class_new_instance(0, NULL, stream_klass);

	Stream_Internal * internal;
	Data_Get_Struct(self, Stream_Internal, internal);

	internal->stream = stream;
	internal->format = format;
	internal->metadata = av_dictionary_to_ruby_hash(internal->stream->metadata);

	return self;
}

/*
**
*/
VALUE stream_format(VALUE self) {
	Stream_Internal * internal;
	Data_Get_Struct(self, Stream_Internal, internal);

	return internal->format;
}

/*
**
*/
VALUE stream_index(VALUE self) {
	Stream_Internal * internal;
	Data_Get_Struct(self, Stream_Internal, internal);

	return INT2NUM(internal->stream->index);
}

/*
**
*/
VALUE stream_type(VALUE self) {
	Stream_Internal * internal;
	Data_Get_Struct(self, Stream_Internal, internal);

	switch (internal->stream->codec->codec_type) {
		case AVMEDIA_TYPE_VIDEO:
			return ID2SYM(rb_intern("video"));
		case AVMEDIA_TYPE_AUDIO:
			return ID2SYM(rb_intern("audio"));
		case AVMEDIA_TYPE_DATA:
			return ID2SYM(rb_intern("data"));
		case AVMEDIA_TYPE_SUBTITLE:
			return ID2SYM(rb_intern("subtitle"));
		case AVMEDIA_TYPE_ATTACHMENT:
			return ID2SYM(rb_intern("attachment"));
		default:
			return ID2SYM(rb_intern("unknown"));
	}
}

/*
**
*/
VALUE stream_tag(VALUE self) {
	Stream_Internal * internal;
	Data_Get_Struct(self, Stream_Internal, internal);

	char tag[4] = { internal->stream->codec->codec_tag >>  0 & 0xff,
		 			internal->stream->codec->codec_tag >>  8 & 0xff,
					internal->stream->codec->codec_tag >> 16 & 0xff,
					internal->stream->codec->codec_tag >> 24 & 0xff };

	return rb_str_new(tag, 4);
}

/*
**
*/
VALUE stream_metadata(VALUE self) {
	Stream_Internal * internal;
	Data_Get_Struct(self, Stream_Internal, internal);
	
	return internal->metadata;
}

/*
**
*/
VALUE stream_start_time(VALUE self) {
	Stream_Internal * internal;
	Data_Get_Struct(self, Stream_Internal, internal);

	return rb_float_new((double)internal->stream->start_time * (double)internal->stream->time_base.num / (double)internal->stream->time_base.den);
}

/*
**
*/
VALUE stream_duration(VALUE self) {
	Stream_Internal * internal;
	Data_Get_Struct(self, Stream_Internal, internal);

	return rb_float_new((double)internal->stream->duration * (double)internal->stream->time_base.num / (double)internal->stream->time_base.den);
}

/*
**
*/
VALUE stream_frame_count(VALUE self) {
	Stream_Internal * internal;
	Data_Get_Struct(self, Stream_Internal, internal);

	return INT2NUM(internal->stream->nb_frames);
}
