#!/usr/bin/env ruby -W0

require 'ruby-ffmpeg'
require 'gd2-ffij'

File.open("./test/test-1.avi") do |input|
  FFMPEG::Reader.open(input) do |reader|
    # Find first video stream
    first_video_stream = reader.streams.select { |s| s.type == :video }.first
    raise "File does not contain a video stream" unless first_video_stream

    # Decode all frames
    while frame = first_video_stream.decode do
      # Write PNG
      gd2_image = GD2::Image.load(frame.to_gd2)
      gd2_image.export("./output-%03.3f.png" % frame.timestamp)
    end
  end
end
