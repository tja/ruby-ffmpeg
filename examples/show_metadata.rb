#!/usr/bin/env ruby -W0

require 'tja_ffmpeg'

FFMPEG::Format.open(File.open("./test/test.mov")) do |format|
  # global metadata
  format.metadata.each do |key, value|
    puts "#{key} : #{value}"
  end

  # streams
  format.streams.each do |stream|
    puts "", "Stream #{stream.index} (#{stream.type})"
    # stream metadata
    stream.metadata.each do |key, value|
      puts "  #{key} : #{value}"
    end
  end
end
