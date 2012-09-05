#include <ruby.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include "tja_ffmpeg_format.h"
#include "tja_ffmpeg_stream.h"

/*
** Deinitialize.
*/
void DeInit_tja_ffmpeg(VALUE unused) {
	/* nothing right now */
}

/*
** Initialize.
*/
void Init_tja_ffmpeg(void) {
	// Global setup
	av_register_all();
	av_log_set_level(AV_LOG_QUIET);

	rb_set_end_proc(DeInit_tja_ffmpeg, 0);

	// FFMPEG module
	VALUE module = rb_define_module("FFMPEG");

	// Format class
	format_klass = rb_define_class_under(module, "Format", rb_cObject);
	rb_define_alloc_func(format_klass, format_alloc);

	rb_define_const (format_klass, "VERSION",		rb_str_new2(format_version_string()));
	rb_define_const (format_klass, "CONFIGURATION",	rb_str_new2(avformat_configuration()));
	rb_define_const (format_klass, "LICENSE",		rb_str_new2(avformat_license()));

	rb_define_method(format_klass, "initialize",	format_initialize, 1);

	rb_define_method(format_klass, "name", 			format_name, 0);
	rb_define_method(format_klass, "description", 	format_description, 0);
	rb_define_method(format_klass, "start_time", 	format_start_time, 0);
	rb_define_method(format_klass, "duration", 		format_duration, 0);
	rb_define_method(format_klass, "streams", 		format_streams, 0);
	rb_define_method(format_klass, "metadata", 		format_metadata, 0);

	// Stream class
	stream_klass = rb_define_class_under(module, "Stream", rb_cObject);
	rb_define_alloc_func(stream_klass, stream_alloc);

	rb_define_method(stream_klass, "format", 		stream_format, 0);
	rb_define_method(stream_klass, "index", 		stream_index, 0);
	rb_define_method(stream_klass, "type", 			stream_type, 0);
	rb_define_method(stream_klass, "tag", 			stream_tag, 0);
	rb_define_method(stream_klass, "metadata", 		stream_metadata, 0);
	rb_define_method(stream_klass, "start_time", 	stream_start_time, 0);
	rb_define_method(stream_klass, "duration", 		stream_duration, 0);
	rb_define_method(stream_klass, "frame_count", 	stream_frame_count, 0);
}
