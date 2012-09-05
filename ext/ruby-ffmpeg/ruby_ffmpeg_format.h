#ifndef	RUBY_FFMPEG_FORMAT_H
#define	RUBY_FFMPEG_FORMAT_H

#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

// Object Lifetime
VALUE format_register_class(VALUE module);

// Helper Functions
int format_find_next_stream_packet(VALUE self, AVPacket * packet, int stream_index);


#endif // RUBY_FFMPEG_FORMAT_H
