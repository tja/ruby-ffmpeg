#include "ruby_ffmpeg.h"
#include "ruby_ffmpeg_stream.h"
#include "ruby_ffmpeg_stream_private.h"
#include "ruby_ffmpeg_video_frame.h"
#include "ruby_ffmpeg_audio_frame.h"
#include "ruby_ffmpeg_frame.h"
#include "ruby_ffmpeg_util.h"

// Globals
static VALUE _klass;


/*
**	Object Lifetime.
*/

// Register class
VALUE stream_register_class(VALUE module, VALUE super) {
	_klass = rb_define_class_under(module, "Stream", super);
	rb_define_alloc_func(_klass, stream_alloc);

	rb_define_method(_klass, "reader", 			stream_reader, 0);
	rb_define_method(_klass, "index", 			stream_index, 0);
	rb_define_method(_klass, "type", 			stream_type, 0);
	rb_define_method(_klass, "tag", 			stream_tag, 0);
	rb_define_method(_klass, "start_time", 		stream_start_time, 0);
	rb_define_method(_klass, "duration", 		stream_duration, 0);
	rb_define_method(_klass, "frame_count", 	stream_frame_count, 0);
	rb_define_method(_klass, "bit_rate", 		stream_bit_rate, 0);

	rb_define_method(_klass, "metadata", 		stream_metadata, 0);

	// Register sub classes
	video_stream_register_class(module, _klass);
	audio_stream_register_class(module, _klass);

	return _klass;
}

// Allocate object
VALUE stream_alloc(VALUE klass) {
	StreamInternal * internal = (StreamInternal *)av_mallocz(sizeof(StreamInternal));
	if (!internal) rb_raise(rb_eNoMemError, "Failed to allocate internal structure");

	return Data_Wrap_Struct(klass, stream_mark, stream_free, (void *)internal);
}

// Free object
void stream_free(void * opaque) {
	StreamInternal * internal = (StreamInternal *)opaque;
	if (internal) {
		av_free(internal);
	}
}

// Mark for garbage collection
void stream_mark(void * opaque) {
	StreamInternal * internal = (StreamInternal *)opaque;
	if (internal) {
		rb_gc_mark(internal->reader);
		rb_gc_mark(internal->metadata);
	}
}

// Create new instance for given FFMPEG stream
VALUE stream_new(VALUE reader, AVStream * stream) {
	VALUE self = rb_class_new_instance(0, NULL, _klass);

	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	internal->stream = stream;
	internal->reader = reader;
	internal->metadata = av_dictionary_to_ruby_hash(internal->stream->metadata);

	return self;
}


/*
**	Properties.
*/

// Point back to reader
VALUE stream_reader(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	return internal->reader;
}

// Index in media file
VALUE stream_index(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	return INT2NUM(internal->stream->index);
}

// Stream type, nil if unknown
VALUE stream_type(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	return av_media_type_to_symbol(internal->stream->codec->codec_type);
}

// Codec tag
VALUE stream_tag(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	char tag[4] = { internal->stream->codec->codec_tag >>  0 & 0xff,
		 			internal->stream->codec->codec_tag >>  8 & 0xff,
					internal->stream->codec->codec_tag >> 16 & 0xff,
					internal->stream->codec->codec_tag >> 24 & 0xff };

	return rb_str_new(tag, 4);
}

// Start time (in seconds)
VALUE stream_start_time(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	return (internal->stream->start_time != AV_NOPTS_VALUE) ? rb_float_new(internal->stream->start_time * av_q2d(internal->stream->time_base)) : Qnil;
}

// Duration (in seconds)
VALUE stream_duration(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	return (internal->stream->duration != AV_NOPTS_VALUE) ? rb_float_new(internal->stream->duration * av_q2d(internal->stream->time_base)) : Qnil;
}

// Number of frames
VALUE stream_frame_count(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	return INT2NUM(internal->stream->nb_frames);
}

// Bit rate (bits per second)
VALUE stream_bit_rate(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	return INT2NUM(internal->stream->codec->bit_rate);
}

// Metadata                                            
VALUE stream_metadata(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	return internal->metadata;
}
