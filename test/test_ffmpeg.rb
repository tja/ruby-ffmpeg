require 'test/unit'
require 'ruby-ffmpeg'
require 'open-uri'

class FFMPEGTest < Test::Unit::TestCase
  def test_versions
    assert_equal FFMPEG::Reader::VERSION, "54.17.100"
  end

  def test_reader_properties
    File.open("./test/test-1.avi") do |io|
      FFMPEG::Reader.open(io) do |reader|
        assert_equal "avi",               reader.name
        assert_equal "AVI format",        reader.description
        assert_equal "0.000000",          "%.6f" % reader.start_time
        assert_equal "00:00:52.208",      Time.at(reader.duration).utc.strftime('%T.%L')
        assert_equal 126688,              reader.bit_rate
        assert_equal 2,                   reader.streams.length
        assert_equal 4,                   reader.metadata.length
      end
    end
  end

  def test_video_stream_properties
    File.open("./test/test-1.avi") do |io|
      FFMPEG::Reader.open(io) do |reader|
        assert                            reader.streams[0].kind_of? FFMPEG::VideoStream

        assert_equal 0,                   reader.streams[0].index
        assert_equal :video,              reader.streams[0].type
        assert_equal "avc1",              reader.streams[0].tag
        assert_equal "0.000000",          "%.6f" % reader.streams[0].start_time
        assert_equal "00:00:52.208",      Time.at(reader.streams[0].duration).utc.strftime('%T.%L')
        assert_equal :yuv420p,            reader.streams[0].format
        assert_equal 1253,                reader.streams[0].frame_count
        assert_equal 0,                   reader.streams[0].bit_rate      # why?
          
        assert_equal 854,                 reader.streams[0].width
        assert_equal 480,                 reader.streams[0].height
        assert_equal nil,                 reader.streams[0].aspect_ratio
        assert_equal 24.0,                reader.streams[0].frame_rate
          
        assert_equal 0,                   reader.streams[0].metadata.length
      end
    end
  end

  def test_audio_stream_properties
    File.open("./test/test-1.avi") do |io|
      FFMPEG::Reader.open(io) do |reader|
        assert                            reader.streams[1].kind_of? FFMPEG::AudioStream

        assert_equal 1,                   reader.streams[1].index
        assert_equal :audio,              reader.streams[1].type
        assert_equal "\xFF\x00\x00\x00",  reader.streams[1].tag
        assert_equal "0.000000",          "%.6f" % reader.streams[1].start_time
        assert_equal "00:00:51.946",      Time.at(reader.streams[1].duration).utc.strftime('%T.%L')
        assert_equal :s16,                reader.streams[1].format
        assert_equal 2435,                reader.streams[1].frame_count
        assert_equal 126688,              reader.streams[1].bit_rate
        
        assert_equal 2,                   reader.streams[1].channels
        assert_equal "stereo",            reader.streams[1].channel_layout
        assert_equal 48000,               reader.streams[1].sample_rate
        
        assert_equal 0,                   reader.streams[1].metadata.length
      end
    end
  end

  def test_video_frame
    File.open("./test/test-1.avi") do |io|
      FFMPEG::Reader.open(io) do |reader|
        video_frame = reader.streams[0].decode

        assert                            video_frame.kind_of? FFMPEG::VideoFrame

        assert_equal 614880,              video_frame.data.length
        assert_equal "0.041667",          "%.6f" % video_frame.timestamp
        assert_equal "0.020833",          "%.6f" % video_frame.duration
        assert_equal :yuv420p,            video_frame.format
          
        assert_equal 854,                 video_frame.width
        assert_equal 480,                 video_frame.height
        assert_equal 0.0,                 video_frame.aspect_ratio
        assert_equal :i,                  video_frame.picture_type
        assert_equal true,                video_frame.key?
      end
    end
  end
  
  def test_audio_frame
    File.open("./test/test-1.avi") do |io|
      FFMPEG::Reader.open(io) do |reader|
        audio_frame = reader.streams[1].decode

        assert                            audio_frame.kind_of? FFMPEG::AudioFrame

        assert_equal 4096,                audio_frame.data.length
        assert_equal "0.000000",          "%.6f" % audio_frame.timestamp
        assert_equal "0.000021",          "%.6f" % audio_frame.duration
        assert_equal :s16,                audio_frame.format

        assert_equal 2,                   audio_frame.channels
        assert_equal "stereo",            audio_frame.channel_layout
        assert_equal 1024,                audio_frame.samples
        assert_equal 48000,               audio_frame.sample_rate
      end
    end
  end

  def test_video_frame_resample_percentage
    File.open("./test/test-1.avi") do |io|
      FFMPEG::Reader.open(io) do |reader|
        video_frame = reader.streams[0].decode.resample(0.5)

        assert_equal 153840,              video_frame.data.length
        assert_equal "0.041667",          "%.6f" % video_frame.timestamp
        assert_equal "0.020833",          "%.6f" % video_frame.duration
        assert_equal :yuv420p,            video_frame.format
          
        assert_equal 427,                 video_frame.width
        assert_equal 240,                 video_frame.height
        assert_equal 0.0,                 video_frame.aspect_ratio
        assert_equal :i,                  video_frame.picture_type
        assert_equal true,                video_frame.key?
      end
    end
  end
  
  def test_video_frame_resample_format
    File.open("./test/test-1.avi") do |io|
      FFMPEG::Reader.open(io) do |reader|
        video_frame = reader.streams[0].decode.resample(:rgb24)

        assert_equal 1229760,             video_frame.data.length
        assert_equal "0.041667",          "%.6f" % video_frame.timestamp
        assert_equal "0.020833",          "%.6f" % video_frame.duration
        assert_equal :rgb24,              video_frame.format
          
        assert_equal 854,                 video_frame.width
        assert_equal 480,                 video_frame.height
        assert_equal 0.0,                 video_frame.aspect_ratio
        assert_equal :i,                  video_frame.picture_type
        assert_equal true,                video_frame.key?
      end
    end
  end
  
  def test_video_frame_resample_width_height
    File.open("./test/test-1.avi") do |io|
      FFMPEG::Reader.open(io) do |reader|
        video_frame = reader.streams[0].decode.resample(100, 100)

        assert_equal 15000,               video_frame.data.length
        assert_equal "0.041667",          "%.6f" % video_frame.timestamp
        assert_equal "0.020833",          "%.6f" % video_frame.duration
        assert_equal :yuv420p,            video_frame.format
          
        assert_equal 100,                 video_frame.width
        assert_equal 100,                 video_frame.height
        assert_equal 0.0,                 video_frame.aspect_ratio
        assert_equal :i,                  video_frame.picture_type
        assert_equal true,                video_frame.key?
      end
    end
  end

  def test_video_frame_resample_width_height_filter
    File.open("./test/test-1.avi") do |io|
      FFMPEG::Reader.open(io) do |reader|
        video_frame = reader.streams[0].decode.resample(150, 150, :bicubic)

        assert_equal 33750,               video_frame.data.length
        assert_equal "0.041667",          "%.6f" % video_frame.timestamp
        assert_equal "0.020833",          "%.6f" % video_frame.duration
        assert_equal :yuv420p,            video_frame.format
          
        assert_equal 150,                 video_frame.width
        assert_equal 150,                 video_frame.height
        assert_equal 0.0,                 video_frame.aspect_ratio
        assert_equal :i,                  video_frame.picture_type
        assert_equal true,                video_frame.key?
      end
    end
  end

  def test_video_frame_resample_width_height_filter_format
    File.open("./test/test-1.avi") do |io|
      FFMPEG::Reader.open(io) do |reader|
        video_frame = reader.streams[0].decode.resample(200, 200, :bicubic, :rgba)

        assert_equal 160000,              video_frame.data.length
        assert_equal "0.041667",          "%.6f" % video_frame.timestamp
        assert_equal "0.020833",          "%.6f" % video_frame.duration
        assert_equal :rgba,               video_frame.format
          
        assert_equal 200,                 video_frame.width
        assert_equal 200,                 video_frame.height
        assert_equal 0.0,                 video_frame.aspect_ratio
        assert_equal :i,                  video_frame.picture_type
        assert_equal true,                video_frame.key?
      end
    end
  end
end