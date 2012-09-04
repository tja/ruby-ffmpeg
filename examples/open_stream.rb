##!/usr/bin/env ruby -W0

require 'tja_ffmpeg'

FFMPEG::Format.open(File.open("./test/test.mov")) do |format|
  puts "Format: #{format.description}"
end