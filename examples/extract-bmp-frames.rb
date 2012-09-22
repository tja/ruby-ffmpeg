#!/usr/bin/env ruby -W0

require 'ruby-ffmpeg'

File.open("./test/test-1.avi") do |input|
  FFMPEG::Reader.open(input) do |reader|
    # Find first video stream
    first_video_stream = reader.streams.select { |s| s.type == :video }.first
    raise "File does not contain a video stream" unless first_video_stream

    # Create resampler to change color format to BGR24
    resampler = first_video_stream.resampler(:bgr24)

    # Decode all frames
    while frame = first_video_stream.decode do
      # Extract raw data of resampled frame, aligned to 4-byte boundaries
      raw_data = (resampler ^ frame).data(4)

      # Write BMP
      File.open("./output-%03.3f.bmp" % frame.timestamp, "wb") do |output|
        # Header
        header = [ "BM",                    # Magic Number
                   54 + raw_data.length,    # Size of BMP file
                   0,                       # Unused
                   0,                       # Unused
                   54,                      # Offset to pixel array
                   40,                      # Size of DIB Header
                   frame.width,             # Width of bitmap
                   -frame.height,           # Height of bitmap (make negative to flip)
                   1,                       # Number of planes
                   24,                      # Depth per pixel
                   0,                       # BI_RGB - no compression
                   raw_data.length,         # Size of pixel data
                   2835,                    # Horizontal resolution
                   2835,                    # Vertical resolution
                   0,                       # Number of colors
                   0 ]                      # Number of important colors

        output.write(header.pack("A2L<S<S<L<L<L<L<S<S<L<L<L<L<L<L<"))

        # Image Data
        output.write(raw_data)
      end
    end
  end
end
