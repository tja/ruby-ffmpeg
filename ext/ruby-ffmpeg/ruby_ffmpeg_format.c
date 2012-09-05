#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

#include "ruby_ffmpeg_format.h"
#include "ruby_ffmpeg_format_private.h"
#include "ruby_ffmpeg_stream.h"
#include "ruby_ffmpeg_util.h"

// Globals
static VALUE _klass;


/*
**	Object Lifetime.
*/

// Register class
VALUE format_register_class(VALUE module) {
	_klass = rb_define_class_under(module, "Format", rb_cObject);
	rb_define_alloc_func(_klass, format_alloc);

	rb_define_const (_klass, "VERSION",			rb_str_new2(human_readable_version()));
	rb_define_const (_klass, "CONFIGURATION",	rb_str_new2(avformat_configuration()));
	rb_define_const (_klass, "LICENSE",			rb_str_new2(avformat_license()));

	rb_define_method(_klass, "initialize",		format_initialize, 1);

	rb_define_method(_klass, "name", 			format_name, 0);
	rb_define_method(_klass, "description", 	format_description, 0);
	rb_define_method(_klass, "start_time", 		format_start_time, 0);
	rb_define_method(_klass, "duration", 		format_duration, 0);
	rb_define_method(_klass, "bit_rate", 		format_bit_rate, 0);
	rb_define_method(_klass, "streams", 		format_streams, 0);
	rb_define_method(_klass, "metadata", 		format_metadata, 0);
	
	return _klass;
}

// Allocate object
VALUE format_alloc(VALUE klass) {
	FormatInternal * internal = (FormatInternal *)av_mallocz(sizeof(FormatInternal));
	if (!internal) rb_raise(rb_eNoMemError, "Failed to allocate internal structure");

	internal->format = avformat_alloc_context();
	if (!internal->format) rb_raise(rb_eNoMemError, "Failed to allocate FFMPEG format context");

	internal->protocol = avio_alloc_context(av_malloc(FORMAT_READ_BUFFER_SIZE), FORMAT_READ_BUFFER_SIZE, 0, internal, read_packet, NULL, NULL);
	if (!internal->protocol) rb_raise(rb_eNoMemError, "Failed to allocate FFMPEG IO context");

	internal->protocol->seekable = 0;
	internal->format->pb = internal->protocol;

	return Data_Wrap_Struct(klass, format_mark, format_free, (void *)internal);
}

// Free object
void format_free(void * opaque) {
	FormatInternal * internal = (FormatInternal *)opaque;
	if (internal) {
		if (internal->format)
			avformat_free_context(internal->format);
		if (internal->protocol)
			av_free(internal->protocol);
		av_free(internal);
	}
}

// Mark for garbage collection
void format_mark(void * opaque) {
	FormatInternal * internal = (FormatInternal *)opaque;
	if (internal) {
		rb_gc_mark(internal->io);
		rb_gc_mark(internal->streams);
		rb_gc_mark(internal->metadata);
	}
}

// Initialize object
VALUE format_initialize(VALUE self, VALUE io) {
	FormatInternal * internal;
	Data_Get_Struct(self, FormatInternal, internal);

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
VALUE format_name(VALUE self) {
	FormatInternal * internal;
	Data_Get_Struct(self, FormatInternal, internal);
	
	return rb_str_new2(internal->format->iformat->name);
}

// Description
VALUE format_description(VALUE self) {
	FormatInternal * internal;
	Data_Get_Struct(self, FormatInternal, internal);
	
	return rb_str_new2(internal->format->iformat->long_name);
}

// Start time (in seconds)
VALUE format_start_time(VALUE self) {
	FormatInternal * internal;
	Data_Get_Struct(self, FormatInternal, internal);
	
	return rb_float_new(internal->format->start_time / (double)AV_TIME_BASE);
}

// Duration (in seconds)
VALUE format_duration(VALUE self) {
	FormatInternal * internal;
	Data_Get_Struct(self, FormatInternal, internal);
	
	return rb_float_new(internal->format->duration / (double)AV_TIME_BASE);
}

// Bit rate (in bits per second)
VALUE format_bit_rate(VALUE self) {
	FormatInternal * internal;
	Data_Get_Struct(self, FormatInternal, internal);

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
VALUE format_streams(VALUE self) {
	FormatInternal * internal;
	Data_Get_Struct(self, FormatInternal, internal);
	
	return internal->streams;
}

// Metadata
VALUE format_metadata(VALUE self) {
	FormatInternal * internal;
	Data_Get_Struct(self, FormatInternal, internal);
	
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
		rb_ary_push(streams, stream_new(self, format->streams[i]));
	}

	return streams;
}

// Read next block of data
int read_packet(void * opaque, uint8_t * buffer, int buffer_size) {
	FormatInternal * internal = (FormatInternal *)opaque;

	VALUE string = rb_funcall(internal->io, rb_intern("read"), 1, INT2FIX(buffer_size));
	Check_Type(string, T_STRING);
	
	memcpy(buffer, RSTRING_PTR(string), RSTRING_LEN(string));
	return RSTRING_LEN(string);
}

// Find the next packet for the stream
int format_find_next_stream_packet(VALUE self, AVPacket * packet, int stream_index) {
	FormatInternal * internal;
	Data_Get_Struct(self, FormatInternal, internal);

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
