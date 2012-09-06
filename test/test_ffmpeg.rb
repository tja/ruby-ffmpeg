require 'test/unit'
require 'ruby-ffmpeg'
require 'open-uri'

class FFMPEGTest < Test::Unit::TestCase
  def test_versions
    assert_equal FFMPEG::Format::VERSION, "54.17.100"
  end

  def test_format
    File.open("./test/test-1.avi") do |io|
      FFMPEG::Format.open(io) do |format|
        assert_equal format.name, "avi"
        assert_equal format.description, "AVI format"
      end
    end
  end

  def test_format_properties
    File.open("./test/test-1.avi") do |io|
      FFMPEG::Format.open(io) do |format|
        assert_equal "0.000000", "%.6f" % format.start_time
        assert_equal "00:00:52.208", Time.at(format.duration).utc.strftime('%T.%L')
        assert_equal 2, format.streams.length
      end
    end
  end

  def test_sandbox
    File.open("./test/test-2.mp4") do |io|
      FFMPEG::Format.open(io) do |format|
        first_video_stream = format.streams.select { |s| s.type == :video }.first
        if first_video_stream
          10.times do
            frame = first_video_stream.decode nil
            next unless frame

            puts
            puts "*** Decoded Frame"
            puts "    Timestamp:      #{frame.timestamp}"
            puts "    Duration:       #{frame.duration}"
            puts "    Key Frame:      #{frame.key?}"
            puts "    Resolution:     #{frame.width}x#{frame.height}"
            puts "    Picture Type:   #{frame.picture_type}"
            puts "    Interlaced:     #{frame.interlaced?} (TFF: #{frame.top_field_first?})"
            puts "    Channels:       #{frame.channels}"
            puts "    Channel Layout: #{frame.channel_layout}"
            puts "    Samples:        #{frame.samples}"
            puts "    Sample Rate:    #{frame.sample_rate}"
          end
        end
      end
    end
  end
end