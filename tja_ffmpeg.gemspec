Gem::Specification.new do |s|
  s.name        = "tja_ffmpeg"
  s.version     = "0.1.0"
  s.date        = "2012-09-02"
  s.summary     = "Bindings for FFMPEG"
  s.description = "Will follow soon"
  s.authors     = [ "Thomas Jansen" ]
  s.email       = "thomas@nop.io"
  s.files       = Dir.glob("lib/**/*.rb") + Dir.glob("ext/**/*.{c,h,rb}")
  s.extensions  = [ "ext/tja_ffmpeg/extconf.rb" ]
  s.homepage    = "http://github.com/tja/ffmpeg"
end