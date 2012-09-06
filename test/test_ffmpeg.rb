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
    File.open("./test/test-1.avi") do |io|
      FFMPEG::Format.open(io) do |format|
        first_video_stream = format.streams.select { |s| s.type == :video }.first
        if first_video_stream
          puts
          10.times do
            frame = first_video_stream.decode nil
            if frame
              puts "*** Frame Dimensions: #{frame.width}x#{frame.height} at #{frame.timestamp}"
            end
          end
        end
      end
    end
  end
end