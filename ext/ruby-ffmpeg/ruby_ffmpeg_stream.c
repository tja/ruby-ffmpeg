#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

#include "ruby_ffmpeg_stream.h"
#include "ruby_ffmpeg_stream_private.h"
#include "ruby_ffmpeg_frame.h"
#include "ruby_ffmpeg_util.h"

// Globals
static VALUE _klass;


/*
**	Object Lifetime.
*/

// Register class
VALUE stream_register_class(VALUE module) {
	_klass = rb_define_class_under(module, "Stream", rb_cObject);
	rb_define_alloc_func(_klass, stream_alloc);

	rb_define_method(_klass, "reader", 			stream_reader, 0);
	rb_define_method(_klass, "index", 			stream_index, 0);
	rb_define_method(_klass, "type", 			stream_type, 0);
	rb_define_method(_klass, "tag", 			stream_tag, 0);
	rb_define_method(_klass, "start_time", 		stream_start_time, 0);
	rb_define_method(_klass, "duration", 		stream_duration, 0);
	rb_define_method(_klass, "format", 			stream_format, 0);
	rb_define_method(_klass, "frame_count", 	stream_frame_count, 0);
	rb_define_method(_klass, "bit_rate", 		stream_bit_rate, 0);

	rb_define_method(_klass, "width", 			stream_width, 0);
	rb_define_method(_klass, "height", 			stream_height, 0);
	rb_define_method(_klass, "aspect_ratio",	stream_aspect_ratio, 0);
	rb_define_method(_klass, "frame_rate", 		stream_frame_rate, 0);

	rb_define_method(_klass, "channels",		stream_channels, 0);
	rb_define_method(_klass, "channel_layout",	stream_channel_layout, 0);
	rb_define_method(_klass, "sample_rate",		stream_sample_rate, 0);

	rb_define_method(_klass, "metadata", 		stream_metadata, 0);

	rb_define_method(_klass, "decode",			stream_decode, 1);

	return _klass;
}

// Allocate object
VALUE stream_alloc(VALUE klass) {
	StreamInternal * internal = (StreamInternal *)av_mallocz(sizeof(StreamInternal));
	if (!internal) rb_raise(rb_eNoMemError, "Failed to allocate internal structure");

	return Data_Wrap_Struct(klass, stream_mark, stream_free, (void *)internal);
}

// Free object
void stream_free(void * opaque) {
	StreamInternal * internal = (StreamInternal *)opaque;
	if (internal) {
		av_free(internal);
	}
}

// Mark for garbage collection
void stream_mark(void * opaque) {
	StreamInternal * internal = (StreamInternal *)opaque;
	if (internal) {
		rb_gc_mark(internal->reader);
		rb_gc_mark(internal->metadata);
	}
}

// Create new instance for given FFMPEG stream
VALUE stream_new(VALUE reader, AVStream * stream) {
	VALUE self = rb_class_new_instance(0, NULL, _klass);

	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	internal->stream = stream;
	internal->reader = reader;
	internal->metadata = av_dictionary_to_ruby_hash(internal->stream->metadata);

	return self;
}


/*
**	Properties.
*/

// Point back to reader
VALUE stream_reader(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	return internal->reader;
}

// Index in media file
VALUE stream_index(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	return INT2NUM(internal->stream->index);
}

// Stream type, nil if unknown
VALUE stream_type(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	return av_media_type_to_symbol(internal->stream->codec->codec_type);
}

// Codec tag
VALUE stream_tag(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	char tag[4] = { internal->stream->codec->codec_tag >>  0 & 0xff,
		 			internal->stream->codec->codec_tag >>  8 & 0xff,
					internal->stream->codec->codec_tag >> 16 & 0xff,
					internal->stream->codec->codec_tag >> 24 & 0xff };

	return rb_str_new(tag, 4);
}

// Start time (in seconds)
VALUE stream_start_time(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	return (internal->stream->start_time != AV_NOPTS_VALUE) ? rb_float_new(internal->stream->start_time * av_q2d(internal->stream->time_base)) : Qnil;
}

// Duration (in seconds)
VALUE stream_duration(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	return (internal->stream->duration != AV_NOPTS_VALUE) ? rb_float_new(internal->stream->duration * av_q2d(internal->stream->time_base)) : Qnil;
}

// Format of the frame, nil if not available
VALUE stream_format(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	if (internal->stream->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
		// Video formats
		return av_pixel_format_to_symbol(internal->stream->codec->pix_fmt);
	}
	else if (internal->stream->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
		// Audio formats
		return av_sample_format_to_symbol(internal->stream->codec->sample_fmt);
	}
	else {
		// Unsupported stream type
		return Qnil;
	}
}

// Number of frames
VALUE stream_frame_count(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	return INT2NUM(internal->stream->nb_frames);
}

// Bit rate (bits per second)
VALUE stream_bit_rate(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	return INT2NUM(internal->stream->codec->bit_rate);
}

// Video frame width (in pixels), nil if not available
VALUE stream_width(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	return internal->stream->codec->width ? INT2NUM(internal->stream->codec->width) : Qnil;
}

// Video frame height (in pixels), nil if not available
VALUE stream_height(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	return internal->stream->codec->height ? INT2NUM(internal->stream->codec->height) : Qnil;
}

// Video pixel aspect ratio, nil if not available
VALUE stream_aspect_ratio(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	return internal->stream->codec->sample_aspect_ratio.num ? rb_float_new(av_q2d(internal->stream->codec->sample_aspect_ratio)) : Qnil;
}

// Video frame rate (frames per second), nil if not available
VALUE stream_frame_rate(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	return internal->stream->avg_frame_rate.den ? rb_float_new(av_q2d(internal->stream->avg_frame_rate)) : Qnil;
}

// Number of audio channels, nil if not available
VALUE stream_channels(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	return internal->stream->codec->channels ? INT2NUM(internal->stream->codec->channels) : Qnil;
}

// Layout of the audio channels, nil if not available
VALUE stream_channel_layout(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	if (!internal->stream->codec->channels || !internal->stream->codec->channel_layout)
		return Qnil;

	char temp[64];
	av_get_channel_layout_string(&temp[0], sizeof(temp), internal->stream->codec->channels, internal->stream->codec->channels);
	return rb_str_new2(temp);
}

// Audio sample rate (samples per second), nil if not available
VALUE stream_sample_rate(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	return internal->stream->codec->sample_rate ? INT2NUM(internal->stream->codec->sample_rate) : Qnil;
}

// Metadata
VALUE stream_metadata(VALUE self) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);
	
	return internal->metadata;
}


/*
**	Methods.
*/

// Encode frame and pass to block
VALUE stream_decode(VALUE self, VALUE block) {
	StreamInternal * internal;
	Data_Get_Struct(self, StreamInternal, internal);

	// Only video and audio streams supported as of now
	if (internal->stream->codec->codec_type != AVMEDIA_TYPE_VIDEO && internal->stream->codec->codec_type != AVMEDIA_TYPE_AUDIO) {
		return Qnil;
	}

	prepare_codec(internal);
	AVFrame * frame = avcodec_alloc_frame();

	for (;;) {
		// Find next packet for this stream
		AVPacket packet;
		int found = reader_find_next_stream_packet(internal->reader, &packet, internal->stream->index);
		if (!found) {
			// No more packets
			av_free(frame);
			return Qnil;
		}

		// Decode frame
		int decoded = 0;
		if (internal->stream->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			// Video
		    int err = avcodec_decode_video2(internal->stream->codec, frame, &decoded, &packet);
			if (err < 0) rb_raise(rb_eLoadError, av_error_to_ruby_string(err));
		}
		else {
			// Audio
		    int err = avcodec_decode_audio4(internal->stream->codec, frame, &decoded, &packet);
			if (err < 0) rb_raise(rb_eLoadError, av_error_to_ruby_string(err));
		}
		
		if (decoded) {
			// Full frame decoded
			return frame_new(frame, internal->stream->codec);
		}
	}
}


/*
**	Helper Functions.
*/

// Prepare 
void prepare_codec(StreamInternal * internal) {
	if (!avcodec_is_open(internal->stream->codec)) {
		AVCodec * codec = internal->stream->codec->codec;
		if (!codec) {
			codec = avcodec_find_decoder(internal->stream->codec->codec_id);
		}
		avcodec_open(internal->stream->codec, codec);
	}
}
