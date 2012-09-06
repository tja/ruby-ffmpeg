#!/usr/bin/env ruby -W0

require 'ruby-ffmpeg'

File.open("./test/test-2.mp4") do |io|
  FFMPEG::Reader.open(io) do |reader|
    puts "Format: #{reader.description}"
  end
end
