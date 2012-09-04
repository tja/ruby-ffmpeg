require "tja_ffmpeg/tja_ffmpeg"

module FFMPEG
  class Format
    def self.open(io, &block)
      format = Format.new(io)
      if block
        val = block.call(format)
        # format.close
        val
      else
        format
      end
    end
  end
end