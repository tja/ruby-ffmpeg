#include "ruby_ffmpeg.h"
#include "ruby_ffmpeg_frame.h"
#include "ruby_ffmpeg_frame_private.h"
#include "ruby_ffmpeg_video_frame.h"
#include "ruby_ffmpeg_audio_frame.h"
#include "ruby_ffmpeg_util.h"

// Globals
static VALUE _klass;


/*
**	Object Lifetime.
*/

// Register class
VALUE frame_register_class(VALUE module, VALUE super) {
	_klass = rb_define_class_under(module, "Frame", super);

	rb_define_method(_klass, "raw_data",	frame_raw_data, 0);
	rb_define_method(_klass, "timestamp",	frame_timestamp, 0);
	rb_define_method(_klass, "duration",	frame_duration, 0);
	rb_define_method(_klass, "format",		frame_format, 0);

	// Register sub classes
	video_frame_register_class(module, _klass);
	audio_frame_register_class(module, _klass);

	return _klass;
}


/*
**	Properties.
*/

// Return the raw data (as string), base class always returns nil
VALUE frame_raw_data(VALUE self) {
	return Qnil;
}

// Best effort timestamp (in seconds), base class always returns nil
VALUE frame_timestamp(VALUE self) {
	return Qnil;
}

// Duration of this frame (in seconds), base class always returns nil
VALUE frame_duration(VALUE self) {
	return Qnil;
}

// Format of the frame, base class always returns nil
VALUE frame_format(VALUE self) {
	return Qnil;
}
