#ifndef	RUBY_FFMPEG_AUDIO_STREAM_H
#define	RUBY_FFMPEG_AUDIO_STREAM_H

#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

// Object Lifetime
VALUE audio_stream_register_class(VALUE module, VALUE super);
VALUE audio_stream_new(VALUE reader, AVStream * stream);

#endif // RUBY_FFMPEG_AUDIO_STREAM_H
