#!/usr/bin/env ruby -W0

require 'ruby-ffmpeg'

File.open("./test/test-2.mp4") do |io|
  FFMPEG::Format.open(io) do |format|
    puts "Format: #{format.description}"
  end
end
