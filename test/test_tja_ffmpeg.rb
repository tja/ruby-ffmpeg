require 'test/unit'
require 'tja_ffmpeg'

class FFMPEGTest < Test::Unit::TestCase
  def test_version
    assert_equal "54.17.100", FFMPEG::Format::VERSION
  end

  def test_ffmpeg_open
    FFMPEG::Format.new(File.open("./test/test.mov"))
  end

  def test_dump
#    puts FFMPEG::CONFIGURATION
#    puts FFMPEG::LICENSE
  end
end