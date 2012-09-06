#ifndef	RUBY_FFMPEG_READER_PRIVATE_H
#define	RUBY_FFMPEG_READER_PRIVATE_H

#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

// Definitions
#define reader_READ_BUFFER_SIZE		8192		// Buffer size for reading

// Internal Data
typedef struct {
	AVFormatContext *	format;					// FFMPEG: Format context
	AVIOContext *		protocol;				// FFMPEG: IO context

	VALUE				io;						// Ruby: IO class to read from
	VALUE				streams;				// Ruby: Streams wrapped in Ruby objects
	VALUE				metadata;				// Ruby: Array of metadata
} ReaderInternal;


// Object Lifetime
VALUE reader_alloc(VALUE klass);
void reader_free(void * opaque);
void reader_mark(void * opaque);

VALUE reader_initialize(VALUE self, VALUE io);

// Properties
VALUE reader_name(VALUE self);
VALUE reader_description(VALUE self);
VALUE reader_start_time(VALUE self);
VALUE reader_duration(VALUE self);
VALUE reader_bit_rate(VALUE self);
VALUE reader_streams(VALUE self);
VALUE reader_metadata(VALUE self);

// Helper Functions
char const * human_readable_version();
VALUE streams_to_ruby_array(VALUE self, AVFormatContext * format);

int read_packet(void * opaque, uint8_t * buffer, int buffer_size);

#endif // RUBY_FFMPEG_READER_PRIVATE_H
