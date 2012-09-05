#!/usr/bin/env ruby -W0

require 'ruby-ffmpeg'

puts "Syntax: #{File.basename(__FILE__)} <file1> <file2> ..." if ARGV.empty?

ARGV.each do |path|
  FFMPEG::Format.open(File.open(path)) do |format|
    # Header
    puts "Input #0, #{format.name}, from '#{path}':"

    # Metadata
    puts "  Metadata:"
    format.metadata.each do |key, value|
      puts "    %-15s : %s" % [ key, value ] unless key == :language
    end

    # Misc
    puts "  Duration: %02d:%02d:%02d.%02d, start: %.6f, bitrate: %.f kb/s" % [ (format.duration / 60 / 60).floor,
                                                                               (format.duration / 60).floor % 60,
                                                                               format.duration.floor % 60,
                                                                               (format.duration * 100).floor % 100,
                                                                               format.start_time,
                                                                               format.bit_rate / 1000.0 ]

    # Streams
    format.streams.each do |stream|
      # Stream Header
      print "    Stream #0:#{stream.index}"
      print "(#{stream.metadata[:language]})" if stream.metadata[:language]
      print ": "
      puts  case stream.type
            when :video then "Video: %s, %dx%d, %d kb/s, %.2f fps" % [ stream.tag, stream.width, stream.height, stream.bit_rate / 1000, stream.frame_rate ]
            when :audio then "Audio: %s, %d Hz, %d channels, %d kb/s" % [ stream.tag, stream.sample_rate, stream.channels, stream.bit_rate / 1000 ]
            when :data  then "Data: %s" % stream.tag
            end

      # Stream Metadata
      puts "    Metadata:"
      stream.metadata.each do |key, value|
        puts "      %-15s : %s" % [ key, value ] unless key == :language
      end
    end
  end
end
