#!/usr/bin/env ruby -W0

require 'ruby-ffmpeg'
require 'open-uri'

URI.parse("https://github.com/tja/ruby-ffmpeg/raw/master/test/test-2.mp4").open do |io|
  FFMPEG::Format.open(io) do |format|
    format.streams.each do |stream|
      # Stream Header
      puts  case stream.type
            when :video then "Video: %s, %dx%d, %d kb/s, %.2f fps" % [ stream.tag, stream.width, stream.height, stream.bit_rate / 1000, stream.frame_rate ]
            when :audio then "Audio: %s, %d Hz, %d channels, %d kb/s" % [ stream.tag, stream.sample_rate, stream.channels, stream.bit_rate / 1000 ]
            when :data  then "Data: %s" % stream.tag
            end
    end
  end
end
