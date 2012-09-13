#include "ruby_ffmpeg.h"
#include "ruby_ffmpeg_reader.h"
#include "ruby_ffmpeg_stream.h"
#include "ruby_ffmpeg_frame.h"

/*
**	Deinitialize.
*/
void DeInit_ruby_ffmpeg(VALUE data) {
	(void)data;
}

/*
**	Initialize.
*/
void Init_ruby_ffmpeg_ext(void) {
	// FFMPEG
	av_register_all();
	av_log_set_level(AV_LOG_QUIET);

	// Ruby
	rb_set_end_proc(DeInit_ruby_ffmpeg, Qnil);

	VALUE module = rb_define_module("FFMPEG");

	reader_register_class(module, rb_cObject);
	stream_register_class(module, rb_cObject);
	frame_register_class(module, rb_cObject);
}
