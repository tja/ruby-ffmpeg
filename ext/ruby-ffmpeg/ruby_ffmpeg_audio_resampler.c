#include "ruby_ffmpeg.h"
#include "ruby_ffmpeg_audio_resampler.h"
#include "ruby_ffmpeg_audio_resampler_private.h"
#include "ruby_ffmpeg_audio_frame.h"
#include "ruby_ffmpeg_audio_frame_private.h"
#include "ruby_ffmpeg_util.h"

// Globals
static VALUE _klass;


/*
**	Object Lifetime.
*/

// Register class
VALUE audio_resampler_register_class(VALUE module, VALUE super) {
	_klass = rb_define_class_under(module, "AudioResampler", super);
	rb_define_alloc_func(_klass, audio_resampler_alloc);

	rb_define_method(_klass, "src_channels",	audio_resampler_src_channels, 0);
	rb_define_method(_klass, "src_rate",		audio_resampler_src_rate, 0);
	rb_define_method(_klass, "src_format",		audio_resampler_src_format, 0);
	rb_define_method(_klass, "dst_channels",	audio_resampler_dst_channels, 0);
	rb_define_method(_klass, "dst_rate",		audio_resampler_dst_rate, 0);
	rb_define_method(_klass, "dst_format",		audio_resampler_dst_format, 0);

	rb_define_method(_klass, "initialize",		audio_resampler_initialize, -1);
	rb_define_method(_klass, "resample",		audio_resampler_resample, 1);
	rb_define_method(_klass, "^",				audio_resampler_resample, 1);

	return _klass;
}

// Allocate object
VALUE audio_resampler_alloc(VALUE klass) {
	AudioResamplerInternal * internal = (AudioResamplerInternal *)av_mallocz(sizeof(AudioResamplerInternal));
	if (!internal) rb_raise(rb_eNoMemError, "Failed to allocate internal structure");

	return Data_Wrap_Struct(klass, audio_resampler_mark, audio_resampler_free, (void *)internal);
}

// Free object
void audio_resampler_free(void * opaque) {
	AudioResamplerInternal * internal = (AudioResamplerInternal *)opaque;
	if (internal) {
		if (internal->context)
			audio_resample_close(internal->context);
		av_free(internal);
	}
}

// Mark for garbage collection
void audio_resampler_mark(void * opaque) {
	AudioResamplerInternal * internal = (AudioResamplerInternal *)opaque;
	if (internal) {
		// Nothing yet
	}
}

// Create new instance (from object)
VALUE audio_resampler_new(VALUE object, int argc, VALUE * argv) {
	return rb_class_new_instance2(object, argc, argv, _klass);
}
	

/*
**	Properties.
*/

// Expected input channel count
VALUE audio_resampler_src_channels(VALUE self) {
	AudioResamplerInternal * internal;
	Data_Get_Struct(self, AudioResamplerInternal, internal);

	return INT2NUM(internal->src_channels);
}

// Expected input sample rate
VALUE audio_resampler_src_rate(VALUE self) {
	AudioResamplerInternal * internal;
	Data_Get_Struct(self, AudioResamplerInternal, internal);

	return INT2NUM(internal->src_rate);
}

// Expected input sample format
VALUE audio_resampler_src_format(VALUE self) {
	AudioResamplerInternal * internal;
	Data_Get_Struct(self, AudioResamplerInternal, internal);

	return av_sample_format_to_symbol(internal->src_format);
}

// Resulting output channel count
VALUE audio_resampler_dst_channels(VALUE self) {
	AudioResamplerInternal * internal;
	Data_Get_Struct(self, AudioResamplerInternal, internal);

	return INT2NUM(internal->dst_channels);
}

// Resulting output sample rate
VALUE audio_resampler_dst_rate(VALUE self) {
	AudioResamplerInternal * internal;
	Data_Get_Struct(self, AudioResamplerInternal, internal);

	return INT2NUM(internal->dst_rate);
}

// Resulting output format
VALUE audio_resampler_dst_format(VALUE self) {
	AudioResamplerInternal * internal;
	Data_Get_Struct(self, AudioResamplerInternal, internal);

	return av_sample_format_to_symbol(internal->dst_format);
}


/*
**	Methods.
*/

// Initialize resampler
//
// Generic
//     ::new(src_channels, src_rate, src_format, dst_format)							- Change sample format
//     ::new(src_channels, src_rate, src_format, dst_rate, dst_format)					- Change sample rate and format
//     ::new(src_channels, src_rate, src_format, dst_channels, dst_rate, dst_format)	- Change channel count, sample rate, and format
//
// From Object
//     ::new(source, dst_format)														- Change sample format
//     ::new(source, dst_rate, dst_format)												- Change sample rate and format
//     ::new(source, dst_channels, dst_rate, dst_format)								- Change channel count, sample rate, and format
VALUE audio_resampler_initialize(int argc, VALUE * argv, VALUE self) {
	AudioResamplerInternal * internal;
	Data_Get_Struct(self, AudioResamplerInternal, internal);

	if (argc && TYPE(argv[0]) == T_FIXNUM) {
		// Called generic form
		if      (argc < 4)	rb_raise(rb_eArgError, "Missing argument(s)");
		else if (argc > 6)	rb_raise(rb_eArgError, "Too many arguments");

		internal->src_channels 	= NUM2INT(argv[0]);
		internal->src_rate		= NUM2INT(argv[1]);
		internal->src_format	= symbol_to_av_sample_format(argv[2]);
		
		argc -= 3;
		argv += 3;
	}
	else {
		// Called with object
		if 		(argc < 2)	rb_raise(rb_eArgError, "Missing argument(s)");
		else if (argc > 4)	rb_raise(rb_eArgError, "Too many arguments");

		internal->src_channels 	= NUM2INT(rb_funcall(argv[0], rb_intern("channels"), 0));
		internal->src_rate		= NUM2INT(rb_funcall(argv[0], rb_intern("rate"), 0));
		internal->src_format	= symbol_to_av_sample_format(rb_funcall(argv[0], rb_intern("format"), 0));
		
		argc -= 1;
		argv += 1;
	}

	internal->dst_channels	= internal->src_channels;
	internal->dst_rate		= internal->src_rate;
	internal->dst_format	= internal->src_format;

	switch (argc) {
		case 1: {
			// Change sample format
			internal->dst_format = symbol_to_av_sample_format(argv[0]);
			break;
		}
		case 2: {
			// Change sample rate and format
			internal->dst_rate = NUM2INT(argv[0]);
			internal->dst_format = symbol_to_av_sample_format(argv[1]);
			break;
		}
		case 3: {
			// Change channel count, sample rate, and sampleformat
			internal->dst_channels = NUM2INT(argv[0]);
			internal->dst_rate = NUM2INT(argv[1]);
			internal->dst_format = symbol_to_av_sample_format(argv[2]);
			break;
		}
	}

	if (internal->src_format == AV_SAMPLE_FMT_NONE) rb_raise(rb_eArgError, "Unknown input sample format");
	if (internal->dst_format == AV_SAMPLE_FMT_NONE) rb_raise(rb_eArgError, "Unknown output sample format");

	// Create audio resampler
	internal->context = av_audio_resample_init(internal->dst_channels,
									  		   internal->src_channels,
									  		   internal->dst_rate,
									  		   internal->src_rate,
									  		   internal->dst_format,
									  		   internal->src_format,
										  	   0,
									  		   0,
									  		   1,
									  		   0.0);
	if (!internal->context)
		rb_raise(rb_eRuntimeError, "Failed to create resampling context");

	return self;
}

// Resample image
VALUE audio_resampler_resample(VALUE self, VALUE frame) {
	AudioResamplerInternal * internal;
	Data_Get_Struct(self, AudioResamplerInternal, internal);

	AudioFrameInternal * internal_frame;
	Data_Get_Struct(frame, AudioFrameInternal, internal_frame);

	// Allocate enough memory for output samples
	// (we have to use the max channels, as there seems to be a bug in FFMPEG of overshooting the buffer when downsampling)
	int max_channels = (internal->src_channels > internal->dst_channels) ? internal->src_channels : internal->dst_channels;
	int bytes_per_sample = av_get_bytes_per_sample(internal->dst_format) * max_channels;

	uint8_t * dst_data = (uint8_t *)av_mallocz((internal_frame->samples * internal->dst_rate / internal->src_rate + 32) * bytes_per_sample);
	if (!dst_data) rb_raise(rb_eNoMemError, "Failed to allocate new sample buffer");

	// Resample
	int dst_samples = audio_resample(internal->context,
									 (short *)internal_frame->data,
									 (short *)dst_data,
									 internal_frame->samples);

	// Wrap into Ruby object
	return audio_frame_new2(dst_data,
							internal->dst_channels,
							av_get_default_channel_layout(internal->dst_channels),
							internal->dst_format,
							dst_samples,
							internal->dst_rate,
							internal_frame->timestamp,
							internal_frame->duration);
}
