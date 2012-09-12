#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

#include "ruby_ffmpeg_reader.h"
#include "ruby_ffmpeg_reader_private.h"
#include "ruby_ffmpeg_video_stream.h"
#include "ruby_ffmpeg_audio_stream.h"
#include "ruby_ffmpeg_util.h"

// Globals
static VALUE _klass;


/*
**	Object Lifetime.
*/

// Register class
VALUE reader_register_class(VALUE module, VALUE super) {
	_klass = rb_define_class_under(module, "Reader", super);
	rb_define_alloc_func(_klass, reader_alloc);

	rb_define_const (_klass, "VERSION",			rb_str_new2(human_readable_version()));
	rb_define_const (_klass, "CONFIGURATION",	rb_str_new2(avformat_configuration()));
	rb_define_const (_klass, "LICENSE",			rb_str_new2(avformat_license()));

	rb_define_method(_klass, "initialize",		reader_initialize, 1);

	rb_define_method(_klass, "name", 			reader_name, 0);
	rb_define_method(_klass, "description", 	reader_description, 0);
	rb_define_method(_klass, "start_time", 		reader_start_time, 0);
	rb_define_method(_klass, "duration", 		reader_duration, 0);
	rb_define_method(_klass, "bit_rate", 		reader_bit_rate, 0);
	rb_define_method(_klass, "streams", 		reader_streams, 0);
	rb_define_method(_klass, "metadata", 		reader_metadata, 0);
	
	return _klass;
}

// Allocate object
VALUE reader_alloc(VALUE klass) {
	ReaderInternal * internal = (ReaderInternal *)av_mallocz(sizeof(ReaderInternal));
	if (!internal) rb_raise(rb_eNoMemError, "Failed to allocate internal structure");

	internal->format = avformat_alloc_context();
	if (!internal->format) rb_raise(rb_eNoMemError, "Failed to allocate FFMPEG format context");

	internal->protocol = avio_alloc_context(av_malloc(reader_READ_BUFFER_SIZE), reader_READ_BUFFER_SIZE, 0, internal, read_packet, NULL, NULL);
	if (!internal->protocol) rb_raise(rb_eNoMemError, "Failed to allocate FFMPEG IO context");

	internal->protocol->seekable = 0;
	internal->format->pb = internal->protocol;

	return Data_Wrap_Struct(klass, reader_mark, reader_free, (void *)internal);
}

// Free object
void reader_free(void * opaque) {
	ReaderInternal * internal = (ReaderInternal *)opaque;
	if (internal) {
		if (internal->format)
			avformat_free_context(internal->format);
		if (internal->protocol)
			av_free(internal->protocol);
		av_free(internal);
	}
}

// Mark for garbage collection
void reader_mark(void * opaque) {
	ReaderInternal * internal = (ReaderInternal *)opaque;
	if (internal) {
		rb_gc_mark(internal->io);
		rb_gc_mark(internal->streams);
		rb_gc_mark(internal->metadata);
	}
}

// Initialize object
VALUE reader_initialize(VALUE self, VALUE io) {
	ReaderInternal * internal;
	Data_Get_Struct(self, ReaderInternal, internal);

	internal->io = io;

	// Open file via Ruby stream
	int err = avformat_open_input(&internal->format, "unnamed", NULL, NULL);
	if (err) rb_raise(rb_eLoadError, av_error_to_ruby_string(err));

	// Read in stream information
	err = av_find_stream_info(internal->format);
	if (err < 0) rb_raise(rb_eLoadError, av_error_to_ruby_string(err));

	// Extract properties
	internal->streams = streams_to_ruby_array(self, internal->format);
	internal->metadata = av_dictionary_to_ruby_hash(internal->format->metadata);

	return self;
}


/*
**	Properties.
*/

// Name
VALUE reader_name(VALUE self) {
	ReaderInternal * internal;
	Data_Get_Struct(self, ReaderInternal, internal);
	
	return rb_str_new2(internal->format->iformat->name);
}

// Description
VALUE reader_description(VALUE self) {
	ReaderInternal * internal;
	Data_Get_Struct(self, ReaderInternal, internal);
	
	return rb_str_new2(internal->format->iformat->long_name);
}

// Start time (in seconds), nil if not available
VALUE reader_start_time(VALUE self) {
	ReaderInternal * internal;
	Data_Get_Struct(self, ReaderInternal, internal);

	return (internal->format->start_time != AV_NOPTS_VALUE) ? rb_float_new(internal->format->start_time / (double)AV_TIME_BASE) : Qnil;
}

// Duration (in seconds), nil if not available
VALUE reader_duration(VALUE self) {
	ReaderInternal * internal;
	Data_Get_Struct(self, ReaderInternal, internal);
	
	return (internal->format->duration != AV_NOPTS_VALUE) ? rb_float_new(internal->format->duration / (double)AV_TIME_BASE) : Qnil;
}

// Bit rate (in bits per second)
VALUE reader_bit_rate(VALUE self) {
	ReaderInternal * internal;
	Data_Get_Struct(self, ReaderInternal, internal);

	// We don't have a file size, and therefore not direct bit rate
	// Instead, we iterate through all streams and add them up
	int aggregate_bitrate = 0;

	unsigned i = 0;
	for(; i < internal->format->nb_streams; ++i) {
		aggregate_bitrate += internal->format->streams[i]->codec->bit_rate;
	}

	return INT2NUM(aggregate_bitrate);
}

// Media streams
VALUE reader_streams(VALUE self) {
	ReaderInternal * internal;
	Data_Get_Struct(self, ReaderInternal, internal);

	return internal->streams;
}

// Metadata
VALUE reader_metadata(VALUE self) {
	ReaderInternal * internal;
	Data_Get_Struct(self, ReaderInternal, internal);
	
	return internal->metadata;
}


/*
**	Helper Functions.
*/

// Human-readable AVFormat version
char const * human_readable_version() {
	static char version[256];
	snprintf(&version[0], sizeof(version), "%d.%d.%d", (avformat_version() >> 16) & 0xffff,
													   (avformat_version() >>  8) & 0x00ff,
													   (avformat_version()      ) & 0x00ff);
	return version;
}

// Wrap streams in Ruby objects
VALUE streams_to_ruby_array(VALUE self, AVFormatContext * format) {
	VALUE streams = rb_ary_new();

	unsigned i = 0;
	for(; i < format->nb_streams; ++i) {
		switch (format->streams[i]->codec->codec_type) {
			case AVMEDIA_TYPE_VIDEO: {
				// Video stream
				rb_ary_push(streams, video_stream_new(self, format->streams[i]));
				break;
			}
			case AVMEDIA_TYPE_AUDIO: {
				// Audio stream
				rb_ary_push(streams, audio_stream_new(self, format->streams[i]));
				break;
			}
			default: {
				// All other streams
				rb_ary_push(streams, stream_new(self, format->streams[i]));
				break;
			}
		}
	}

	return streams;
}

// Read next block of data
int read_packet(void * opaque, uint8_t * buffer, int buffer_size) {
	ReaderInternal * internal = (ReaderInternal *)opaque;

	VALUE string = rb_funcall(internal->io, rb_intern("read"), 1, INT2FIX(buffer_size));
	if (TYPE(string) == T_NIL) return 0;

	Check_Type(string, T_STRING);

	memcpy(buffer, RSTRING_PTR(string), RSTRING_LEN(string));
	return RSTRING_LEN(string);
}

// Find the next packet for the stream
int reader_find_next_stream_packet(VALUE self, AVPacket * packet, int stream_index) {
	ReaderInternal * internal;
	Data_Get_Struct(self, ReaderInternal, internal);

	for (;;) {
		int err = av_read_frame(internal->format, packet);
		if (err < 0) {
			return 0;
		}

		if (packet->stream_index == stream_index) {
			return 1;
		}
	}
}
