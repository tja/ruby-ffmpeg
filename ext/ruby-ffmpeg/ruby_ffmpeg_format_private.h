#ifndef	RUBY_FFMPEG_FORMAT_PRIVATE_H
#define	RUBY_FFMPEG_FORMAT_PRIVATE_H

#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

// Definitions
#define FORMAT_READ_BUFFER_SIZE		8192		// Buffer size for reading

// Internal Data
typedef struct {
	AVFormatContext *	format;					// FFMPEG: Format context
	AVIOContext *		protocol;				// FFMPEG: IO context

	VALUE				io;						// Ruby: IO class to read from
	VALUE				streams;				// Ruby: Streams wrapped in Ruby objects
	VALUE				metadata;				// Ruby: Array of metadata
} FormatInternal;


// Object Lifetime
VALUE format_alloc(VALUE klass);
void format_free(void * opaque);
void format_mark(void * opaque);

VALUE format_initialize(VALUE self, VALUE io);

// Properties
VALUE format_name(VALUE self);
VALUE format_description(VALUE self);
VALUE format_start_time(VALUE self);
VALUE format_duration(VALUE self);
VALUE format_streams(VALUE self);
VALUE format_bit_rate(VALUE self);
VALUE format_metadata(VALUE self);

// Helper Functions
char const * human_readable_version();
VALUE streams_to_ruby_array(VALUE self, AVFormatContext * format);

int read_packet(void * opaque, uint8_t * buffer, int buffer_size);

#endif // RUBY_FFMPEG_FORMAT_PRIVATE_H
