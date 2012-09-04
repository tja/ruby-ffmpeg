#include <ruby.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include "tja_ffmpeg_format.h"

#define FORMAT_READ_BUFFER_SIZE		8192

// Internal data
typedef struct {
	// LibAV
	AVFormatContext *	format;
	AVIOContext *		io;
	// Ruby
	VALUE				stream;
} Format_Internal;

/*
**
*/
char const * format_version_string()
{
	static char version[256];
	snprintf(&version[0], sizeof(version), "%d.%d.%d", (avformat_version() >> 16) & 0xffff,
													   (avformat_version() >>  8) & 0x00ff,
													   (avformat_version()      ) & 0x00ff);
	return version;
}

/*
**
*/
int read_packet(void * opaque, uint8_t * buffer, int buffer_size)
{
	Format_Internal * internal = (Format_Internal *)opaque;

	VALUE string = rb_funcall(internal->stream, rb_intern("read"), 1, INT2FIX(buffer_size));
	Check_Type(string, T_STRING);
	
	memcpy(buffer, RSTRING_PTR(string), RSTRING_LEN(string));
	return RSTRING_LEN(string);
}

/*
**
*/
void klass_free(void * opaque)
{
	Format_Internal * internal = (Format_Internal *)opaque;
	if (internal) {
		if (internal->format)
			avformat_free_context(internal->format);
		if (internal->io)
			av_free(internal->io);
		av_free(internal);
	}
}

/*
**
*/
void klass_mark(void * opaque)
{
	Format_Internal * internal = (Format_Internal *)opaque;
	if (internal) {
		rb_gc_mark(internal->stream);
	}
}

/*
**
*/
VALUE format_alloc(VALUE klass)
{
	Format_Internal * internal = (Format_Internal *)av_mallocz(sizeof(Format_Internal));
	if (!internal) rb_raise(rb_eNoMemError, "Filed to allocate internal structure");

	internal->io = avio_alloc_context(av_malloc(FORMAT_READ_BUFFER_SIZE), FORMAT_READ_BUFFER_SIZE, 0, internal, read_packet, NULL, NULL);
	if (!internal->io) rb_raise(rb_eNoMemError, "Failed to allocate FFMPEG IO context");
	internal->io->seekable = 0;

	internal->format = avformat_alloc_context();
	if (!internal->format) rb_raise(rb_eNoMemError, "Failed to allocate FFMPEG format context");

	internal->format->pb = internal->io;

	return Data_Wrap_Struct(klass, klass_mark, klass_free, (void *)internal);
}

/*
**
*/
VALUE format_initialize(VALUE self, VALUE stream)
{
	Format_Internal * internal;
	Data_Get_Struct(self, Format_Internal, internal);

	internal->stream = stream;

	int err = avformat_open_input(&internal->format, "unnamed", NULL, NULL);
	if (err) {
		char strerror[1024];
		av_strerror(err, &strerror[0], sizeof(strerror));
		rb_raise(rb_eLoadError, strerror);
	}

	printf("Successfully detected file format: %s\n", internal->format->iformat->name);
	return self;
}
