#!/usr/bin/env ruby -W0

require 'ruby-ffmpeg'

File.open("./test/test-2.mp4") do |io|
  FFMPEG::Reader.open(io) do |reader|
    first_audio_stream = reader.streams.select { |s| s.type == :audio }.first
    if first_audio_stream
      File.open("./output.raw", "wb") do |file|
        while frame = first_audio_stream.decode do
          file.write(frame.raw_data)
        end
      end
    end
  end
end
