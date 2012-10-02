#!/usr/bin/env ruby -W0

require 'ruby-ffmpeg'
require 'gd2-ffij'

File.open("./test/test-2.mp4") do |input|
  FFMPEG::Reader.open(input) do |reader|
    # Find first video stream
    first_video_stream = reader.streams.select { |s| s.type == :video }.first
    raise "File does not contain a video stream" unless first_video_stream

    # Create black image to "darken" out a bottom row
    darken = GD2::Image::TrueColor.new(first_video_stream.width, 22)
    darken.draw do |canvas|
      canvas.color = GD2::Color[0.0, 0.0, 0.0]
      canvas.rectangle(0.0, 0.0, darken.width, darken.height, true)
    end

    # Decode frames
    while frame = first_video_stream.decode do
      # Convert to GD2
      image = GD2::Image.load(frame.to_gd2)

      # Darken out background
      image.merge_from(darken, 0, image.height - darken.height, 0, 0, darken.width, darken.height, 0.5)

      # Put in timer string
      image.draw do |canvas|
        canvas.color = GD2::Color[1.0, 1.0, 1.0]
        canvas.font = GD2::Font::Small
        canvas.move_to(image.width / 2.0 - 36.0, image.height - 18)
        canvas.text(Time.at(frame.timestamp).utc.strftime('%H:%M:%S.%L'))
      end

      # Export as PNG
      image.export("./output-%03.3f.png" % frame.timestamp)
    end
  end
end
