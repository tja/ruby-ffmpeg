require "tja_ffmpeg/tja_ffmpeg"

module FFMPEG

  def open(io, &block)
    ffmpeg = FFMPEG.new(io)
    if block
      val = block.call(ffmpeg)
      ffmpeg.close
      val
    else
      ffmpeg
    end
  end

  class FFMPEG
    # def self.version
    # end
  end
end