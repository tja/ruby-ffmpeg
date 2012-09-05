#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

#include "ruby_ffmpeg_frame.h"
#include "ruby_ffmpeg_frame_private.h"

// Globals
static VALUE _klass;


/*
**	Object Lifetime.
*/

// Register class
VALUE frame_register_class(VALUE module) {
	_klass = rb_define_class_under(module, "Frame", rb_cObject);
	rb_define_alloc_func(_klass, frame_alloc);

	rb_define_method(_klass, "width",	frame_width, 0);
	rb_define_method(_klass, "height",	frame_height, 0);

	return _klass;
}

// Allocate object
VALUE frame_alloc(VALUE klass) {
	FrameInternal * internal = (FrameInternal *)av_mallocz(sizeof(FrameInternal));
	if (!internal) rb_raise(rb_eNoMemError, "Failed to allocate internal structure");

	return Data_Wrap_Struct(klass, frame_mark, frame_free, (void *)internal);
}

// Free object
void frame_free(void * opaque) {
	FrameInternal * internal = (FrameInternal *)opaque;
	if (internal) {
		if (internal->frame)
			av_free(internal->frame);
		av_free(internal);
	}
}

// Mark for garbage collection
void frame_mark(void * opaque) {
	FrameInternal * internal = (FrameInternal *)opaque;
	if (internal) {
		// Nothing right now
	}
}

// Create new instance for given FFMPEG frame
VALUE frame_new(AVFrame * frame) {
	VALUE self = rb_class_new_instance(0, NULL, _klass);

	FrameInternal * internal;
	Data_Get_Struct(self, FrameInternal, internal);

	internal->frame = frame;

	return self;
}


/*
**	Properties.
*/

// Video frame width (in pixels), nil if not available
VALUE frame_width(VALUE self) {
	FrameInternal * internal;
	Data_Get_Struct(self, FrameInternal, internal);

	return internal->frame->width ? INT2NUM(internal->frame->width) : Qnil;
}

// Video frame height (in pixels), nil if not available
VALUE frame_height(VALUE self) {
	FrameInternal * internal;
	Data_Get_Struct(self, FrameInternal, internal);

	return internal->frame->height ? INT2NUM(internal->frame->height) : Qnil;
}
