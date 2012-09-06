require "ruby-ffmpeg/ruby_ffmpeg_ext"

module FFMPEG
  class Reader
    def self.open(io, &block)
      reader = Reader.new(io)
      if block
        val = block.call(reader)
        # reader.close
        val
      else
        reader
      end
    end
  end
end