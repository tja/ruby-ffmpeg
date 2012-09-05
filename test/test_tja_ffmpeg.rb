require 'test/unit'
require 'tja_ffmpeg'

class FFMPEGTest < Test::Unit::TestCase
  def test_versions
    assert_equal FFMPEG::Format::VERSION, "54.17.100"
  end

  def test_format
    FFMPEG::Format.open(File.open("./test/test_1.mov")) do |format|
      assert_equal format.name, "mov,mp4,m4a,3gp,3g2,mj2"
      assert_equal format.description, "QuickTime/MPEG-4/Motion JPEG 2000 format"
    end
  end

  def test_format_properties
    FFMPEG::Format.open(File.open("./test/test_1.mov")) do |format|
      assert Time.at(format.start_time).utc.strftime('%T.%L'), "00:00:00.000"
      assert Time.at(format.duration  ).utc.strftime('%T.%L'), "00:01:31.466"
    end
  end

  def test_sandbox
  end
end