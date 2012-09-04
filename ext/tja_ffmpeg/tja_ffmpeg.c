#include <ruby.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include "tja_ffmpeg_format.h"

/*
** Deinitialize.
*/
void DeInit_tja_ffmpeg(VALUE unused)
{
	/* nothing right now */
}

/*
** Initialize.
*/
void Init_tja_ffmpeg(void)
{
	VALUE module, format_klass;

	// Set up FFMPEG
	av_register_all();
	av_log_set_level(AV_LOG_QUIET);

	// FFMPEG module
	module = rb_define_module("FFMPEG");

	// Format class
	format_klass = rb_define_class_under(module, "Format", rb_cObject);
	rb_define_alloc_func(format_klass, format_alloc);

	rb_define_method(format_klass, "initialize",	format_initialize, 1);

	rb_define_const(format_klass, "VERSION",		rb_str_new2(format_version_string()));
	rb_define_const(format_klass, "CONFIGURATION",	rb_str_new2(avformat_configuration()));
	rb_define_const(format_klass, "LICENSE",		rb_str_new2(avformat_license()));

	// ...
	// rb_define_singleton_method(klass, "version_int", ffmpeg_version_int, 0);
	// rb_define_singleton_method(klass, "configuration", ffmpeg_configuration, 0);
	// rb_define_singleton_method(klass, "license", ffmpeg_license, 0);

	rb_set_end_proc(DeInit_tja_ffmpeg, 0);
}
