#ifndef	RUBY_FFMPEG_READER_H
#define	RUBY_FFMPEG_READER_H

#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

// Object Lifetime
VALUE reader_register_class(VALUE module);

// Helper Functions
int reader_find_next_stream_packet(VALUE self, AVPacket * packet, int stream_index);


#endif // RUBY_FFMPEG_READER_H
