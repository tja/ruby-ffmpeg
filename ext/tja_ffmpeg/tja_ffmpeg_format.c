#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

#include "tja_ffmpeg_format.h"
#include "tja_ffmpeg_stream.h"
#include "tja_ffmpeg_util.h"

#define FORMAT_READ_BUFFER_SIZE		8192

// ...
VALUE format_klass;

// Internal data
typedef struct {
	// LibAV
	AVFormatContext *	format;
	AVIOContext *		protocol;
	// Ruby
	VALUE				io;
	VALUE				streams;
	VALUE				metadata;
} Format_Internal;

/*
**
*/
VALUE extract_streams(VALUE self, AVFormatContext * format) {
	VALUE streams = rb_ary_new();

	unsigned i = 0;
	for(; i < format->nb_streams; ++i) {
		rb_ary_push(streams, stream_create_instance(self, format->streams[i]));
	}

	return streams;
}

/*
**
*/
int read_packet(void * opaque, uint8_t * buffer, int buffer_size) {
	Format_Internal * internal = (Format_Internal *)opaque;

	VALUE string = rb_funcall(internal->io, rb_intern("read"), 1, INT2FIX(buffer_size));
	Check_Type(string, T_STRING);
	
	memcpy(buffer, RSTRING_PTR(string), RSTRING_LEN(string));
	return RSTRING_LEN(string);
}

/*
**
*/
void format_mark(void * opaque) {
	Format_Internal * internal = (Format_Internal *)opaque;
	if (internal) {
		rb_gc_mark(internal->io);
		rb_gc_mark(internal->streams);
		rb_gc_mark(internal->metadata);
	}
}

/*
**
*/
void format_free(void * opaque) {
	Format_Internal * internal = (Format_Internal *)opaque;
	if (internal) {
		if (internal->format)
			avformat_free_context(internal->format);
		if (internal->protocol)
			av_free(internal->protocol);
		av_free(internal);
	}
}

/*
**
*/
VALUE format_alloc(VALUE klass) {
	Format_Internal * internal = (Format_Internal *)av_mallocz(sizeof(Format_Internal));
	if (!internal) rb_raise(rb_eNoMemError, "Filed to allocate internal structure");

	internal->format = avformat_alloc_context();
	if (!internal->format) rb_raise(rb_eNoMemError, "Failed to allocate FFMPEG format context");

	internal->protocol = avio_alloc_context(av_malloc(FORMAT_READ_BUFFER_SIZE), FORMAT_READ_BUFFER_SIZE, 0, internal, read_packet, NULL, NULL);
	if (!internal->protocol) rb_raise(rb_eNoMemError, "Failed to allocate FFMPEG IO context");

	internal->protocol->seekable = 0;
	internal->format->pb = internal->protocol;

	return Data_Wrap_Struct(klass, format_mark, format_free, (void *)internal);
}

/*
**
*/
VALUE format_initialize(VALUE self, VALUE io) {
	Format_Internal * internal;
	Data_Get_Struct(self, Format_Internal, internal);

	internal->io = io;

	// Open file via Ruby stream
	int err = avformat_open_input(&internal->format, "unnamed", NULL, NULL);
	if (err) {
		char strerror[1024];
		av_strerror(err, &strerror[0], sizeof(strerror));
		rb_raise(rb_eLoadError, strerror);
	}

	// Read in stream information
	err = av_find_stream_info(internal->format);
	if (err < 0) {
		char strerror[1024];
		av_strerror(err, &strerror[0], sizeof(strerror));
		rb_raise(rb_eLoadError, strerror);
	}

	// Extract properties
	internal->streams = extract_streams(self, internal->format);
	internal->metadata = av_dictionary_to_ruby_hash(internal->format->metadata);

	return self;
}

/*
**
*/
VALUE format_name(VALUE self) {
	Format_Internal * internal;
	Data_Get_Struct(self, Format_Internal, internal);
	
	return rb_str_new2(internal->format->iformat->name);
}

/*
**
*/
VALUE format_description(VALUE self) {
	Format_Internal * internal;
	Data_Get_Struct(self, Format_Internal, internal);
	
	return rb_str_new2(internal->format->iformat->long_name);
}

/*
**
*/
VALUE format_start_time(VALUE self) {
	Format_Internal * internal;
	Data_Get_Struct(self, Format_Internal, internal);
	
	return rb_float_new(internal->format->start_time / (double)AV_TIME_BASE);
}

/*
**
*/
VALUE format_duration(VALUE self) {
	Format_Internal * internal;
	Data_Get_Struct(self, Format_Internal, internal);
	
	return rb_float_new(internal->format->duration / (double)AV_TIME_BASE);
}

/*
**
*/
VALUE format_streams(VALUE self) {
	Format_Internal * internal;
	Data_Get_Struct(self, Format_Internal, internal);
	
	return internal->streams;
}

/*
**
*/
VALUE format_bit_rate(VALUE self) {
	Format_Internal * internal;
	Data_Get_Struct(self, Format_Internal, internal);

	// We don't have a file size, and therefore not direct bit rate
	// Instead, we iterate through all streams and add them up
	int aggregate_bitrate = 0;

	unsigned i = 0;
	for(; i < internal->format->nb_streams; ++i) {
		aggregate_bitrate += internal->format->streams[i]->codec->bit_rate;
	}

	return INT2NUM(aggregate_bitrate);
}

/*
**
*/
VALUE format_metadata(VALUE self) {
	Format_Internal * internal;
	Data_Get_Struct(self, Format_Internal, internal);
	
	return internal->metadata;
}

/*
**
*/
char const * format_version_string() {
	static char version[256];
	snprintf(&version[0], sizeof(version), "%d.%d.%d", (avformat_version() >> 16) & 0xffff,
													   (avformat_version() >>  8) & 0x00ff,
													   (avformat_version()      ) & 0x00ff);
	return version;
}
