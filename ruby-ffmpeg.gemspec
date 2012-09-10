Gem::Specification.new do |s|
  s.name        = "ruby-ffmpeg"
  s.version     = "0.1.0"
  s.date        = "2012-09-02"
  s.summary     = "Ruby bindings for FFMPEG libraries"
  s.description = "ruby-ffmpeg is a Ruby extension for the FFMPEG set of libraries; libavformat, libavcodec, libavutil, and libswscale."
  s.authors     = [ "Thomas Jansen" ]
  s.email       = "thomas@nop.io"
  s.files       = Dir.glob("lib/**/*.rb") + Dir.glob("ext/**/*.{c,h,rb}")
  s.extensions  = [ "ext/ruby-ffmpeg/extconf.rb" ]
  s.homepage    = "http://github.com/tja/ruby-ffmpeg"
end