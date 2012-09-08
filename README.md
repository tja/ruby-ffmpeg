ruby-ffmpeg
===========
Ruby bindings for [FFMPEG](http://ffmpeg.org) libraries.

Summary
-------
ruby-ffmpeg is a Ruby extension for the FFMPEG set of libraries; libavformat, libavcodec, and libavutil.

Installation
------------
Download and build the latest version of FFMPEG. For instance, [MacPorts](http://www.macports.org/) can be used for OS X..

    sudo port install ffmpeg-devel +nonfree

ruby-ffmpeg can then be installed on top via..

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

- A lot of functionality is missing
- The API will most certainly change
- *Nothing* will work as expected

TODO List
---------
These are the next items on the agenda..

1. Frame class
   - Split up Frame into VideoFrame and AudioFrame (with Frame as base class)
   - Add resizing and format conversion to VideoFrame
   - Add resampling and format conversion to AudioFrame
2. Subtitle class
   - Make also base on Frame? Should be renamed to "StreamData" or something
3. Add decode_frame method to Reader
   - Also add an each_frame to Stream and Reader
   - Allow to filter for key frames in video streams
4. Codec class
   - Allow enumerating all existing codecs
