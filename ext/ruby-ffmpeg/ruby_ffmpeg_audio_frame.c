#include "ruby_ffmpeg.h"
#include "ruby_ffmpeg_audio_frame.h"
#include "ruby_ffmpeg_audio_frame_private.h"
#include "ruby_ffmpeg_audio_resampler.h"
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

	rb_define_method(_klass, "data",			audio_frame_data, 0);
	rb_define_method(_klass, "timestamp",		audio_frame_timestamp, 0);
	rb_define_method(_klass, "duration",		audio_frame_duration, 0);
	rb_define_method(_klass, "format",			audio_frame_format, 0);

	rb_define_method(_klass, "channels",		audio_frame_channels, 0);
	rb_define_method(_klass, "channel_layout",	audio_frame_channel_layout, 0);
	rb_define_method(_klass, "samples",			audio_frame_samples, 0);
	rb_define_method(_klass, "rate",			audio_frame_rate, 0);

	rb_define_method(_klass, "resampler", 		audio_frame_resampler, -1);
	rb_define_method(_klass, "resample",		audio_frame_resample, 1);
	rb_define_method(_klass, "^",				audio_frame_resample, 1);

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
		if (internal->data)
			av_free(internal->data);
		av_free(internal);
	}
}

// Mark for garbage collection
void audio_frame_mark(void * opaque) {
	AudioFrameInternal * internal = (AudioFrameInternal *)opaque;
	if (internal) {
		rb_gc_mark(internal->timestamp);
		rb_gc_mark(internal->duration);
	}
}

// Create new instance for given FFMPEG frame
VALUE audio_frame_new(AVFrame * frame, AVCodecContext * codec) {
	// Time stamp: start of with presentation timestamp of frame
	int64_t timestamp = frame->pts;
	if (timestamp == (int64_t)AV_NOPTS_VALUE) {
		// Fall back to presentation timestamp of packet
		timestamp = frame->pkt_pts;
		if (timestamp == (int64_t)AV_NOPTS_VALUE) {
			// Fall back to decompression timestamp of packet
			timestamp = frame->pkt_dts;
		}
	}

	int64_t duration = frame->pkt_duration;

	// Copy data into new sample buffer
	int plane_size = 0;
	int data_size = av_samples_get_buffer_size(&plane_size, codec->channels, codec->frame_size, codec->sample_fmt, 1);
	int planes = av_sample_fmt_is_planar(codec->sample_fmt) ? codec->channels : 1;

	uint8_t * buffer = av_malloc(data_size);
	if (!buffer) rb_raise(rb_eNoMemError, "Failed to allocate sample buffer");

	int i;
	for (i = 0; i < planes; ++i) {
		memcpy(buffer + i * plane_size, frame->extended_data[i], plane_size);
	}

	// Clean up
	av_free(frame);

	// Call main init method
	return audio_frame_new2(buffer,
		                  	codec->channels,
						    codec->channel_layout,
						  	codec->sample_fmt,
						  	codec->frame_size,
						  	codec->sample_rate,
						  	(timestamp != (int64_t)AV_NOPTS_VALUE) ? rb_float_new(timestamp * av_q2d(codec->time_base)) : Qnil,
						  	(duration != (int64_t)AV_NOPTS_VALUE) ? rb_float_new(duration * av_q2d(codec->time_base)) : Qnil);
}

// Create new instance
VALUE audio_frame_new2(uint8_t * data, int channels, uint64_t channel_layout, enum AVSampleFormat format, int samples, int rate, VALUE timestamp, VALUE duration) {
	VALUE self = rb_class_new_instance(0, NULL, _klass);

	AudioFrameInternal * internal;
	Data_Get_Struct(self, AudioFrameInternal, internal);

	internal->data = data;

	internal->channels = channels;
	internal->channel_layout = channel_layout;
	internal->format = format;
	internal->samples = samples;
	internal->rate = rate;

	internal->timestamp = timestamp;
	internal->duration = duration;

	return self;
}


/*
**	Properties.
*/

// Return the raw data (as string)
VALUE audio_frame_data(VALUE self) {
	AudioFrameInternal * internal;
	Data_Get_Struct(self, AudioFrameInternal, internal);

	int bytes_per_sample = av_get_bytes_per_sample(internal->format);
	return rb_str_new((char const *)internal->data, bytes_per_sample * internal->channels * internal->samples);
}

// Best effort timestamp (in seconds), nil if not available
VALUE audio_frame_timestamp(VALUE self) {
	AudioFrameInternal * internal;
	Data_Get_Struct(self, AudioFrameInternal, internal);

	return internal->timestamp;
}

// Duration of this frame (in seconds), nil if not available
VALUE audio_frame_duration(VALUE self) {
	AudioFrameInternal * internal;
	Data_Get_Struct(self, AudioFrameInternal, internal);

	return internal->duration;
}

// Format of the frame, nil if not available
VALUE audio_frame_format(VALUE self) {
	AudioFrameInternal * internal;
	Data_Get_Struct(self, AudioFrameInternal, internal);

	return av_sample_format_to_symbol(internal->format);
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
	av_get_channel_layout_string(&temp[0], sizeof(temp), internal->channels, internal->channel_layout);
	return rb_str_new2(temp);
}

// Number of audio samples in this frame
VALUE audio_frame_samples(VALUE self) {
	AudioFrameInternal * internal;
	Data_Get_Struct(self, AudioFrameInternal, internal);

	return INT2NUM(internal->samples);
}

// Audio sample rate (samples per second)
VALUE audio_frame_rate(VALUE self) {
	AudioFrameInternal * internal;
	Data_Get_Struct(self, AudioFrameInternal, internal);

	return INT2NUM(internal->rate);
}


/*
**	Methods.
*/

// Create resampler for object
VALUE audio_frame_resampler(int argc, VALUE * argv, VALUE self) {
	return audio_resampler_new(self, argc, argv);
}

// Resample audio frame with given resampler
VALUE audio_frame_resample(VALUE self, VALUE resampler) {
	return rb_funcall(resampler, rb_intern("resample"), 1, self);
}
