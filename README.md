ruby-ffmpeg
===========
Ruby bindings for FFMPEG libraries.

Summary
-------
ruby-ffmpeg is a Ruby extension for the FFMPEG set of libraries, including libavformat, libavcodec, and libavutil.

Installation
------------
Download and build the latest version of FFMPEG..
  
    sudo port install ffmpeg-devel +nonfree

You can now continue with ffmpeg-ruby.

    git clone git://github.com/tja/ruby-ffmpeg.git
	cd ruby-ffmpeg
    gem build ruby-ffmpeg.gemspec
    sudo gem install ruby-ffmpeg*.gem

Documentation
-------------
