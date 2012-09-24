#!/usr/bin/env ruby -W0

require 'ruby-ffmpeg'

File.open("./test/test-1.avi") do |input|
  FFMPEG::Reader.open(input) do |reader|
    # Find first video stream
    first_video_stream = reader.streams.select { |s| s.type == :video }.first
    raise "File does not contain a video stream" unless first_video_stream

    # Decode all frames
    while frame = first_video_stream.decode do
      # Write BMP
      File.open("./output-%03.3f.bmp" % frame.timestamp, "wb") do |output|
        output.write(frame.to_bmp)
      end
    end
  end
end
