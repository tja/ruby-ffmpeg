# ruby-ffmpeg

Ruby bindings for [FFMPEG](http://ffmpeg.org) libraries.

## Summary

*ruby-ffmpeg* is a Ruby extension for the FFMPEG set of libraries; libavformat, libavcodec, libavutil, and libswscale.

In contrast to many other FFMPEG-related gems, *ruby-ffmpeg* has been implemented as a C extension that calls into FFMPEG's set of libraries and is not a wrapper around the command line version. This allows much tighter integration with Ruby, finer control of the decoding process, and more efficient access of the media data. One example of such tight integration is that *ruby-ffmpeg* works with every IO object, from files, to string buffers, and even HTTP streams.

Some of *ruby-ffmpeg*'s main features are:

* Data can be read from every Ruby IO object, e.g. files, string buffers, and HTTP streams
* Access to all important properties of streams, video frames, and audio frames
* Supports decoding of video and audio stream (subtitles will come soon)
* Supports resizing of video frames and converting to different color spaces

## Prerequisites

*ruby-ffmpeg* requires the FFMPEG libraries to be installed on your system.

### MacPorts

On OS X, [MacPorts](http://www.macports.org/) can be used to download and build the latest version of FFMPEG:

    sudo port install ffmpeg-devel

### Ubuntu

The base version of FFMPEG can be installed via:

    sudo apt-get install ffmpeg

Consult the the [Ubuntu Community Help Wiki](https://help.ubuntu.com/community/FFmpeg) for details.

### From Source

FFMPEG can be installed from source like this:

    git clone git://source.ffmpeg.org/ffmpeg.git ffmpeg
	cd ffmpeg
	./configure
	make
	make install

The [FFMPEG Trac Wiki](https://ffmpeg.org/trac/ffmpeg/wiki/CompilationGuide) provided more information.

## Installation

Once FFMPEG is available on the system, *ruby-ffmpeg* can be installed on top:

    git clone git://github.com/tja/ruby-ffmpeg.git
	cd ruby-ffmpeg
    gem build ruby-ffmpeg.gemspec
    sudo gem install ruby-ffmpeg*.gem

## Documentation

A very simple example to extract the first frame of a video and store it as RGB raw data:

```ruby
require 'ruby-ffmpeg'

File.open("/path/to/video.mp4") do |io|
  FFMPEG::Reader.open(io) do |reader|
    video_stream = reader.streams.select { |s| s.type == :video }.first
    first_frame_as_rgb24 = video_stream.decode.resample(:rgb24)
	File.open("/path/to/output.raw", "wb") { |f| f.write(frame.data) }
  end
end
```

More tutorials and a full API documentation will come soon, but steer over to the [examples](https://github.com/tja/ruby-ffmpeg/tree/master/examples) folder for now.

## Status

*ruby-ffmpeg* is work-in-progress, which means:

* A lot of the functionality is still missing
* The API will most certainly change
* I am sure there are plenty of bugs

### Short-Term Todo List

I am currently working on these things:

1. Resampling
   * Resampling moves into it's own class for efficiency
   * VideoStream#resampler will return a VideoResampler object
   * VideoResampler#[vf] and VideoFrame#resample(vr) will allow to resample video frames
   * AudioStream#resampler will return an AudioResampler object
   * AudioResampler#[af] and AudioFrame#resample(ar) will allow to resample audio frames
2. Import and Export to other libraries
  * Add support for GD format for gd2 support
  * Add raw support for RMagick
  * Example: Export frames and store as JPG in gd2
  * Example: Export frames and store as JPG in RMagick
  * Anything with sound?
3. Subtitle decoding
   * Add SubtitleStream
   * Add SubtitleFrame
4. Add Reader#decode
   * Also add Stream#each\_frame and Reader#each\_frame
   * Allow to filter for key frames in #decode and #each_frame
5. Create own codec class
   * Allow enumerating all existing codecs
