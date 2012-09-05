require 'test/unit'
require 'ruby-ffmpeg'
require 'open-uri'

class FFMPEGTest < Test::Unit::TestCase
  def test_versions
    assert_equal FFMPEG::Format::VERSION, "54.17.100"
  end

  def test_format
    File.open("./test/test-1.mp4") do |io|
      FFMPEG::Format.open(io) do |format|
        assert_equal format.name, "mov,mp4,m4a,3gp,3g2,mj2"
        assert_equal format.description, "QuickTime/MPEG-4/Motion JPEG 2000 format"
      end
    end
  end

  def test_format_properties
    File.open("./test/test-1.mp4") do |io|
      FFMPEG::Format.open(io) do |format|
        assert_equal nil, format.start_time
        assert_equal "00:00:52.209", Time.at(format.duration).utc.strftime('%T.%L')
        assert_equal 2, format.streams.length
        assert_equal 664569, format.bit_rate
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
            puts "*** Frame Dimensions: #{frame.width}x#{frame.height}"
          end
        end
      end
    end
  end
end