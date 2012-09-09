#include <ruby.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

#include "ruby_ffmpeg_audio_frame.h"
#include "ruby_ffmpeg_audio_frame_private.h"
#include "ruby_ffmpeg_util.h"

// Globals
static VALUE _klass;


/*
**	Object Lifetime.
*/

// Register class
VALUE audio_frame_register_class(VALUE module, VALUE super) {
	_klass = rb_define_class_under(module, "AudioFrame", super);
	rb_define_alloc_func(_klass, audio_frame_alloc);

	rb_define_method(_klass, "raw_data",		audio_frame_raw_data, 0);
	rb_define_method(_klass, "timestamp",		audio_frame_timestamp, 0);
	rb_define_method(_klass, "duration",		audio_frame_duration, 0);
	rb_define_method(_klass, "format",			audio_frame_format, 0);

	rb_define_method(_klass, "channels",		audio_frame_channels, 0);
	rb_define_method(_klass, "channel_layout",	audio_frame_channel_layout, 0);
	rb_define_method(_klass, "samples",			audio_frame_samples, 0);
	rb_define_method(_klass, "sample_rate",		audio_frame_sample_rate, 0);

	return _klass;
}

// Allocate object
VALUE audio_frame_alloc(VALUE klass) {
	AudioFrameInternal * internal = (AudioFrameInternal *)av_mallocz(sizeof(AudioFrameInternal));
	if (!internal) rb_raise(rb_eNoMemError, "Failed to allocate internal structure");

	return Data_Wrap_Struct(klass, audio_frame_mark, audio_frame_free, (void *)internal);
}

// Free object
void audio_frame_free(void * opaque) {
	AudioFrameInternal * internal = (AudioFrameInternal *)opaque;
	if (internal) {
		if (internal->frame)
			av_free(internal->frame);
		av_free(internal);
	}
}

// Mark for garbage collection
void audio_frame_mark(void * opaque) {
	AudioFrameInternal * internal = (AudioFrameInternal *)opaque;
	if (internal) {
		// Nothing right now
	}
}

// Create new instance for given FFMPEG frame
VALUE audio_frame_new(AVFrame * frame, AVCodecContext * codec) {
	VALUE self = rb_class_new_instance(0, NULL, _klass);

	AudioFrameInternal * internal;
	Data_Get_Struct(self, AudioFrameInternal, internal);

	internal->frame = frame;
	internal->time_base = codec->time_base;
	internal->channels = codec->channels;
	internal->channel_layout = codec->channel_layout;

	return self;
}


/*
**	Properties.
*/

// Return the raw data (as string)
VALUE audio_frame_raw_data(VALUE self) {
	AudioFrameInternal * internal;
	Data_Get_Struct(self, AudioFrameInternal, internal);

	// Extract sample data
	int bytes_per_sample = av_get_bytes_per_sample(internal->frame->format);

	if (av_sample_fmt_is_planar(internal->frame->format)) {
		// Planar
		VALUE data = rb_str_new(NULL, 0);

		int i;
		for (i = 0; i < internal->channels; ++i) {
			data = rb_str_cat(data,
							  internal->frame->extended_data[i],
							  bytes_per_sample * internal->frame->nb_samples);
		}

		return data;
	}
	else {
		// Interleaved
		return rb_str_new(internal->frame->extended_data[0],
						  bytes_per_sample * internal->frame->nb_samples * internal->channels);
	}
}

// Best effort timestamp (in seconds), nil if not available
VALUE audio_frame_timestamp(VALUE self) {
	AudioFrameInternal * internal;
	Data_Get_Struct(self, AudioFrameInternal, internal);

	// Start of with best effort
	int64_t timestamp = internal->frame->best_effort_timestamp;
	if (timestamp != AV_NOPTS_VALUE)
		return rb_float_new(timestamp * av_q2d(internal->time_base));

	// Fall back to presentation timestamp of frame
	timestamp = internal->frame->pts;
	if (timestamp != AV_NOPTS_VALUE)
		return rb_float_new(timestamp * av_q2d(internal->time_base));

	// Fall back to presentation timestamp of packet
	timestamp = internal->frame->pkt_pts;
	if (timestamp != AV_NOPTS_VALUE)
		return rb_float_new(timestamp * av_q2d(internal->time_base));

	// Fall back to decompression timestamp of packet
	timestamp = internal->frame->pkt_dts;
	if (timestamp != AV_NOPTS_VALUE)
		return rb_float_new(timestamp * av_q2d(internal->time_base));

	return Qnil;
}

// Duration of this frame (in seconds), nil if not available
VALUE audio_frame_duration(VALUE self) {
	AudioFrameInternal * internal;
	Data_Get_Struct(self, AudioFrameInternal, internal);

	return (internal->frame->pkt_duration != AV_NOPTS_VALUE) ? rb_float_new(internal->frame->pkt_duration * av_q2d(internal->time_base)) : Qnil;
}

// Format of the frame, nil if not available
VALUE audio_frame_format(VALUE self) {
	AudioFrameInternal * internal;
	Data_Get_Struct(self, AudioFrameInternal, internal);

	return av_sample_format_to_symbol(internal->frame->format);
}

// Number of audio channels
VALUE audio_frame_channels(VALUE self) {
	AudioFrameInternal * internal;
	Data_Get_Struct(self, AudioFrameInternal, internal);

	return INT2NUM(internal->channels);
}

// Layout of the audio channels
VALUE audio_frame_channel_layout(VALUE self) {
	AudioFrameInternal * internal;
	Data_Get_Struct(self, AudioFrameInternal, internal);

	char temp[64];
	av_get_channel_layout_string(&temp[0], sizeof(temp), internal->channels, internal->channels);
	return rb_str_new2(temp);
}

// Number of audio samples in this frame
VALUE audio_frame_samples(VALUE self) {
	AudioFrameInternal * internal;
	Data_Get_Struct(self, AudioFrameInternal, internal);

	return INT2NUM(internal->frame->nb_samples);
}

// Audio sample rate (samples per second)
VALUE audio_frame_sample_rate(VALUE self) {
	AudioFrameInternal * internal;
	Data_Get_Struct(self, AudioFrameInternal, internal);

	return INT2NUM(av_frame_get_sample_rate(internal->frame));
}
