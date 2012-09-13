#include "ruby_ffmpeg.h"
#include "ruby_ffmpeg_audio_stream.h"
#include "ruby_ffmpeg_audio_stream_private.h"
#include "ruby_ffmpeg_stream_private.h"
#include "ruby_ffmpeg_audio_frame.h"
#include "ruby_ffmpeg_reader.h"
#include "ruby_ffmpeg_util.h"

// Globals
static VALUE _klass;


/*
**	Object Lifetime.
*/

// Register class
VALUE audio_stream_register_class(VALUE module, VALUE super) {
	_klass = rb_define_class_under(module, "AudioStream", super);
	// rb_define_alloc_func(_klass, audio_stream_alloc);

	rb_define_method(_klass, "type", 			audio_stream_type, 0);
	rb_define_method(_klass, "format", 			audio_stream_format, 0);

	rb_define_method(_klass, "channels",		audio_stream_channels, 0);
	rb_define_method(_klass, "channel_layout",	audio_stream_channel_layout, 0);
	rb_define_method(_klass, "sample_rate",		audio_stream_sample_rate, 0);

	rb_define_method(_klass, "decode",			audio_stream_decode, 0);

	return _klass;
}

// Allocate object
VALUE audio_stream_alloc(VALUE klass) {
	AudioStreamInternal * internal = (AudioStreamInternal *)av_mallocz(sizeof(AudioStreamInternal));
	if (!internal) rb_raise(rb_eNoMemError, "Failed to allocate internal structure");

	return Data_Wrap_Struct(klass, audio_stream_mark, audio_stream_free, (void *)internal);
}

// Free object
void audio_stream_free(void * opaque) {
	AudioStreamInternal * internal = (AudioStreamInternal *)opaque;
	if (internal) {
		stream_free(internal);
	}
}

// Mark for garbage collection
void audio_stream_mark(void * opaque) {
	AudioStreamInternal * internal = (AudioStreamInternal *)opaque;
	if (internal) {
		stream_mark(internal);
	}
}

// Create new instance for given FFMPEG audio stream
VALUE audio_stream_new(VALUE reader, AVStream * stream) {
	VALUE self = rb_class_new_instance(0, NULL, _klass);

	AudioStreamInternal * internal;
	Data_Get_Struct(self, AudioStreamInternal, internal);

	internal->base.stream = stream;
	internal->base.reader = reader;
	internal->base.metadata = av_dictionary_to_ruby_hash(stream->metadata);

	return self;
}


/*
**	Properties.
*/

// Stream type; always returns :audio
VALUE audio_stream_type(VALUE self) {
	(void)self;
	return ID2SYM(rb_intern("audio"));
}

// Format of audio frame
VALUE audio_stream_format(VALUE self) {
	AudioStreamInternal * internal;
	Data_Get_Struct(self, AudioStreamInternal, internal);

	return av_sample_format_to_symbol(internal->base.stream->codec->sample_fmt);
}

// Number of audio channels
VALUE audio_stream_channels(VALUE self) {
	AudioStreamInternal * internal;
	Data_Get_Struct(self, AudioStreamInternal, internal);

	return INT2NUM(internal->base.stream->codec->channels);
}

// Layout of the audio channels
VALUE audio_stream_channel_layout(VALUE self) {
	AudioStreamInternal * internal;
	Data_Get_Struct(self, AudioStreamInternal, internal);

	char temp[64];
	av_get_channel_layout_string(&temp[0], sizeof(temp), internal->base.stream->codec->channels, internal->base.stream->codec->channel_layout);
	return rb_str_new2(temp);
}

// Audio sample rate (samples per second)
VALUE audio_stream_sample_rate(VALUE self) {
	AudioStreamInternal * internal;
	Data_Get_Struct(self, AudioStreamInternal, internal);

	return INT2NUM(internal->base.stream->codec->sample_rate);
}

/*
**	Methods.
*/

// Encode frame and pass to block
VALUE audio_stream_decode(VALUE self) {
	AudioStreamInternal * internal;
	Data_Get_Struct(self, AudioStreamInternal, internal);

	// Prepare codec
	if (!avcodec_is_open(internal->base.stream->codec)) {
		AVCodec * codec = internal->base.stream->codec->codec;
		if (!codec) {
			codec = avcodec_find_decoder(internal->base.stream->codec->codec_id);
		}
		avcodec_open2(internal->base.stream->codec, codec, NULL);
	}

	// Find and decode next audio frame
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

		// Decode audio frame
		int decoded = 0;
	    int err = avcodec_decode_audio4(internal->base.stream->codec, frame, &decoded, &packet);
		if (err < 0) rb_raise_av_error(rb_eLoadError, err);
				
		if (decoded) {
			return audio_frame_new(frame, internal->base.stream->codec);
		}
	}
}
