#!/usr/bin/env ruby -W0

require 'ruby-ffmpeg'

File.open("./test/test-2.mp4") do |io|
  FFMPEG::Reader.open(io) do |reader|
    first_video_stream = reader.streams.select { |s| s.type == :video }.first
    if first_video_stream
      250.times { first_video_stream.decode }
      frame = first_video_stream.decode
      if frame
        File.open("./output.yuv", "wb") { |f| f.write(frame.data) }
        puts "convert -size #{frame.width}x#{frame.height} -depth 8 -interlace plane output.yuv -compress JPEG -quality 90 output.jpg"
      end
    end
  end
end
