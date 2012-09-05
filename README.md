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