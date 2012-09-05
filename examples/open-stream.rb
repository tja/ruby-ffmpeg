#!/usr/bin/env ruby -W0

require 'ruby-ffmpeg'

FFMPEG::Format.open(File.open("./test/test_2.mov")) do |format|
  puts "Format: #{format.description}"
end