#include <ruby.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include "ruby_ffmpeg_reader.h"
#include "ruby_ffmpeg_stream.h"

/*
**	Deinitialize.
*/
void DeInit_ruby_ffmpeg(VALUE unused) {
	/* nothing for now */
}

/*
**	Initialize.
*/
void Init_ruby_ffmpeg_ext(void) {
	// FFMPEG
	av_register_all();
	av_log_set_level(AV_LOG_QUIET);

	// Ruby
	rb_set_end_proc(DeInit_ruby_ffmpeg, 0);

	VALUE module = rb_define_module("FFMPEG");
	reader_register_class(module);
	stream_register_class(module);
	frame_register_class(module);
}
