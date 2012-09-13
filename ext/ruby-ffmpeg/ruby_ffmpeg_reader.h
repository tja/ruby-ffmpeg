#ifndef	RUBY_FFMPEG_READER_H
#define	RUBY_FFMPEG_READER_H

#include "ruby_ffmpeg.h"

// Object Lifetime
VALUE reader_register_class(VALUE module, VALUE super);

// Helper Functions
int reader_find_next_stream_packet(VALUE self, AVPacket * packet, int stream_index);


#endif // RUBY_FFMPEG_READER_H
