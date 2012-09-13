#include "ruby_ffmpeg.h"
#include "ruby_ffmpeg_video_stream.h"
#include "ruby_ffmpeg_video_stream_private.h"
#include "ruby_ffmpeg_stream_private.h"
#include "ruby_ffmpeg_video_frame.h"
#include "ruby_ffmpeg_reader.h"
#include "ruby_ffmpeg_util.h"

// Globals
static VALUE _klass;


/*
**	Object Lifetime.
*/

// Register class
VALUE video_stream_register_class(VALUE module, VALUE super) {
	_klass = rb_define_class_under(module, "VideoStream", super);
	rb_define_alloc_func(_klass, video_stream_alloc);

	rb_define_method(_klass, "type", 			video_stream_type, 0);
	rb_define_method(_klass, "format", 			video_stream_format, 0);

	rb_define_method(_klass, "width", 			video_stream_width, 0);
	rb_define_method(_klass, "height", 			video_stream_height, 0);
	rb_define_method(_klass, "aspect_ratio",	video_stream_aspect_ratio, 0);
	rb_define_method(_klass, "frame_rate", 		video_stream_frame_rate, 0);

	rb_define_method(_klass, "decode",			video_stream_decode, 0);

	return _klass;
}

// Allocate object
VALUE video_stream_alloc(VALUE klass) {
	VideoStreamInternal * internal = (VideoStreamInternal *)av_mallocz(sizeof(VideoStreamInternal));
	if (!internal) rb_raise(rb_eNoMemError, "Failed to allocate internal structure");

	return Data_Wrap_Struct(klass, video_stream_mark, video_stream_free, (void *)internal);
}

// Free object
void video_stream_free(void * opaque) {
	VideoStreamInternal * internal = (VideoStreamInternal *)opaque;
	if (internal) {
		stream_free(internal);
	}
}

// Mark for garbage collection
void video_stream_mark(void * opaque) {
	VideoStreamInternal * internal = (VideoStreamInternal *)opaque;
	if (internal) {
		stream_mark(internal);
	}
}

// Create new instance for given FFMPEG video stream
VALUE video_stream_new(VALUE reader, AVStream * stream) {
	VALUE self = rb_class_new_instance(0, NULL, _klass);

	VideoStreamInternal * internal;
	Data_Get_Struct(self, VideoStreamInternal, internal);

	internal->base.stream = stream;
	internal->base.reader = reader;
	internal->base.metadata = av_dictionary_to_ruby_hash(stream->metadata);

	return self;
}


/*
**	Properties.
*/

// Stream type; always returns :video
VALUE video_stream_type(VALUE self) {
	return ID2SYM(rb_intern("video"));
}

// Format of video frame
VALUE video_stream_format(VALUE self) {
	VideoStreamInternal * internal;
	Data_Get_Struct(self, VideoStreamInternal, internal);
	
	return av_pixel_format_to_symbol(internal->base.stream->codec->pix_fmt);
}

// Video frame width (in pixels)
VALUE video_stream_width(VALUE self) {
	VideoStreamInternal * internal;
	Data_Get_Struct(self, VideoStreamInternal, internal);
	
	return INT2NUM(internal->base.stream->codec->width);
}

// Video frame height (in pixels)
VALUE video_stream_height(VALUE self) {
	VideoStreamInternal * internal;
	Data_Get_Struct(self, VideoStreamInternal, internal);
	
	return INT2NUM(internal->base.stream->codec->height);
}

// Video pixel aspect ratio, nil if not available
VALUE video_stream_aspect_ratio(VALUE self) {
	VideoStreamInternal * internal;
	Data_Get_Struct(self, VideoStreamInternal, internal);
	
	return internal->base.stream->codec->sample_aspect_ratio.num ? rb_float_new(av_q2d(internal->base.stream->codec->sample_aspect_ratio)) : Qnil;
}

// Video frame rate (frames per second)
VALUE video_stream_frame_rate(VALUE self) {
	VideoStreamInternal * internal;
	Data_Get_Struct(self, VideoStreamInternal, internal);
	
	return rb_float_new(av_q2d(internal->base.stream->avg_frame_rate));
}


/*
**	Methods.
*/

// Encode video frame and pass to block
VALUE video_stream_decode(VALUE self) {
	VideoStreamInternal * internal;
	Data_Get_Struct(self, VideoStreamInternal, internal);
	
	// Prepare codec
	if (!avcodec_is_open(internal->base.stream->codec)) {
		AVCodec * codec = internal->base.stream->codec->codec;
		if (!codec) {
			codec = avcodec_find_decoder(internal->base.stream->codec->codec_id);
		}
		avcodec_open2(internal->base.stream->codec, codec, NULL);
	}
	
	// Find and decode next video frame
	AVFrame * frame = avcodec_alloc_frame();
	
	for (;;) {
		// Find next packet for this stream
		AVPacket packet;
		int found = reader_find_next_stream_packet(internal->base.reader, &packet, internal->base.stream->index);
		if (!found) {
			// No more packets
			av_free(frame);
			return Qnil;
		}
	
		// Decode frame
		int decoded = 0;
	    int err = avcodec_decode_video2(internal->base.stream->codec, frame, &decoded, &packet);
		if (err < 0) rb_raise_av_error(rb_eLoadError, err);
	
		if (decoded) {
			return video_frame_new(frame, internal->base.stream->codec);
		}
	}
}
