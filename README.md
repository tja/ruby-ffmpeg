ruby-ffmpeg
===========
Ruby bindings for [FFMPEG](http://ffmpeg.org) libraries.

Summary
-------
*ruby-ffmpeg* is a Ruby extension for the FFMPEG set of libraries; libavformat, libavcodec, libavutil, and libswscale.

In contrast to many other FFMPEG-related gems, *ruby-ffmpeg* has been implemented as a C extension that calls into FFMPEG's set of libraries and is not a wrapper around the command line version. This allows much tighter integration with Ruby, finer control of the decoding process, and more efficient access of the media data. One example of such tight integration is that *ruby-ffmpeg* works with every IO object, from files, to string buffers, and even HTTP streams.

*ruby-ffmpeg*'s main features are:

* Data can be read from every Ruby IO object, e.g. files, string buffers, and HTTP streams
* Access to all important properties of streams, video frames, and audio frames
* Supports decoding of video and audio stream (subtitles will come soon)
* Supports resizing of video frames and converting to different color spaces

Installation
------------
Download and build the latest version of FFMPEG. For instance, [MacPorts](http://www.macports.org/) can be used for OS X..

    sudo port install ffmpeg-devel +nonfree

*ruby-ffmpeg* can then be installed on top via..

    git clone git://github.com/tja/ruby-ffmpeg.git
	cd ruby-ffmpeg
    gem build ruby-ffmpeg.gemspec
    sudo gem install ruby-ffmpeg*.gem

Documentation
-------------
Will come soon.

Status
------
This is heavily in development, which means..

* A lot of functionality is missing
* The API will most certainly change
* __Nothing__ will work as expected

TODO List
---------
These are the next items on the agenda..

1. Add resampling and format conversion to AudioFrame
2. Subtitle class
   * Make also base on Frame? Should be renamed to "StreamData" or something
3. Add decode_frame method to Reader
   * Also add an each_frame to Stream and Reader
   * Allow to filter for key frames in video streams
4. Codec class
   * Allow enumerating all existing codecs
