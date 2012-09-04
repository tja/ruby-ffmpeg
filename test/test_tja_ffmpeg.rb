require 'test/unit'
require 'tja_ffmpeg'

class FFMPEGTest < Test::Unit::TestCase
  def test_version
    assert_equal "54.17.100", FFMPEG::Format::VERSION
  end

  def test_ffmpeg_open
    FFMPEG::Format.open(File.open("./test/test.mov")) do |format|
      assert_equal "mov,mp4,m4a,3gp,3g2,mj2", format.name
      assert_equal "QuickTime/MPEG-4/Motion JPEG 2000 format", format.description
      
      puts "Stream Count: #{format.streams.count}"
      puts format.metadata
      format.streams.each do |stream|
        puts "   #{stream.index} - #{stream.type} - #{stream.tag} - #{stream.start_time} - #{stream.duration} - #{stream.frame_count}"
        puts stream.metadata
      end
    end
  end
end