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

    require 'ruby-ffmpeg'
    
    File.open("/path/to/video.mp4") do |io|
      FFMPEG::Reader.open(io) do |reader|
        video_stream = reader.streams.select { |s| s.type == :video }.first
        first_frame_as_rgb24 = video_stream.decode.resample(:rgb24)
		File.open("/path/to/output.raw", "wb") { |f| f.write(frame.data) }
      end
	end

More tutorials and API documentation will come soon.

## Status

This is heavily in development, which means..

* A lot of functionality is missing
* The API will most certainly change
* __Nothing__ will work as expected

## Todo List

The next items on the agenda are...

1. Add resampling and format conversion to AudioFrame
2. Subtitle class
3. Add decode_frame method to Reader
   * Also add an each_frame to Stream and Reader
   * Allow to filter for key frames in VideoStream and Reader
4. Codec class
   * Create own codec class
   * Allow enumerating all existing codecs
