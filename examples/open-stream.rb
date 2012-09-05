#!/usr/bin/env ruby -W0

require 'ruby-ffmpeg'

FFMPEG::Format.open(File.open("./test/test-2.mp4")) do |format|
  puts "Format: #{format.description}"
end