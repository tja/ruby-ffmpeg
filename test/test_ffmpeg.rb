require 'test/unit'
require 'ruby-ffmpeg'

class FFMPEGTest < Test::Unit::TestCase
  def test_versions
    assert_equal FFMPEG::Format::VERSION, "54.17.100"
  end

  def test_format
    FFMPEG::Format.open(File.open("./test/test-1.mp4")) do |format|
      assert_equal format.name, "mov,mp4,m4a,3gp,3g2,mj2"
      assert_equal format.description, "QuickTime/MPEG-4/Motion JPEG 2000 format"
    end
  end

  def test_format_properties
    FFMPEG::Format.open(File.open("./test/test-1.mp4")) do |format|
      assert_equal nil, format.start_time
      assert_equal "00:00:52.209", Time.at(format.duration  ).utc.strftime('%T.%L')
      assert_equal 2, format.streams.length
      assert_equal 664569, format.bit_rate
    end
  end

  def test_sandbox
  #   FFMPEG::Format.open(File.open("./test/test-2.mp4")) do |format|
  #     first_video_stream = format.streams.select { |s| s.type == :video }.first
  #     if first_video_stream
  #       first_video_stream.decode nil
  #       first_video_stream.decode nil
  #       first_video_stream.decode nil
  #       first_video_stream.decode nil
  #       first_video_stream.decode nil
  #     end
  #   end
  end
end