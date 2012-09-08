require 'test/unit'
require 'ruby-ffmpeg'
require 'open-uri'

class FFMPEGTest < Test::Unit::TestCase
  def test_versions
    assert_equal FFMPEG::Reader::VERSION, "54.17.100"
  end

  def test_reader
    File.open("./test/test-1.avi") do |io|
      FFMPEG::Reader.open(io) do |reader|
        assert_equal "avi", reader.name
        assert_equal "AVI format", reader.description
      end
    end
  end

  def test_reader_properties
    File.open("./test/test-1.avi") do |io|
      FFMPEG::Reader.open(io) do |reader|
        assert_equal "0.000000", "%.6f" % reader.start_time
        assert_equal "00:00:52.208", Time.at(reader.duration).utc.strftime('%T.%L')
        assert_equal 2, reader.streams.length
        assert_equal :yuv420p, reader.streams[0].format
        assert_equal :s16, reader.streams[1].format
      end
    end
  end

  def test_sandbox
    File.open("./test/test-2.mp4") do |io|
      FFMPEG::Reader.open(io) do |reader|
        first_video_stream = reader.streams.select { |s| s.type == :video }.first
        if first_video_stream
          10.times do
            frame = first_video_stream.decode
            next unless frame
    
            puts
            puts "*** Decoded Frame"
            puts "    Data:                 #{(frame.raw_data || "").length}"
            puts "    Timestamp:            #{frame.timestamp}"
            puts "    Duration:             #{frame.duration}"
            puts "    Format:               #{frame.format}"

            if frame.kind_of? FFMPEG::VideoFrame
              puts "    Video Resolution:     #{frame.width}x#{frame.height}"
              puts "    Video Picture Type:   #{frame.picture_type}"
              puts "    Video Interlaced:     #{frame.interlaced?} (TFF: #{frame.top_field_first?})"
              puts "    Video Key Frame:      #{frame.key?}"
            end

            if frame.kind_of? FFMPEG::AudioFrame
              puts "    Audio Channels:       #{frame.channels}"
              puts "    Audio Channel Layout: #{frame.channel_layout}"
              puts "    Audio Samples:        #{frame.samples}"
              puts "    Audio Sample Rate:    #{frame.sample_rate}"
            end
          end
        end
      end
    end
  end
end