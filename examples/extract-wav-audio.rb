#!/usr/bin/env ruby -W0

require 'ruby-ffmpeg'

File.open("./test/test-2.mp4") do |input|
  FFMPEG::Reader.open(input) do |reader|
    # Find first audio stream
    first_audio_stream = reader.streams.select { |s| s.type == :audio }.first
    raise "File does not contain an audio stream" unless first_audio_stream

    # Create resampler to change sample rate to 44.1k and sample format to signed 16bit
    resampler = first_audio_stream.resampler(2, 41000, :s16)

    # Write WAV
    File.open("./output.wav", "wb") do |output|
      # Leave space for header
      output.write(' ' * 44)

      # Decode all frames
      while frame = first_audio_stream.decode do
        # Extract raw data of resampled audio
        raw_data = (resampler ^ frame).data
        output.write(raw_data)
      end

      # Update header in the beginning of the file
      header = [ "RIFF",                  # ChunkID: RIFF
                 output.size - 8,         # ChunkSize: Remaining file size after this value
                 "WAVE",                  # Format: WAVE

                 "fmt ",                  # Subchunk1ID: Format
                 16,                      # Subchunk1Size: 16 bytes
                 1,                       # AudioFormat: PCM = 1
                 2,                       # NumChannels: Stereo
                 44100,                   # SampleRate: 44.1 kHz
                 2 * 44100 * 2,           # ByteRate: 2 channels * 44.1 kHz samples * 2 byte per sample
                 2 * 2,                   # BlockAlign: 2 channels * 2 bytes per sample
                 16,                      # BitsPerSample: 16 bits

                 "data",                  # Subchunk2ID: Data
                 output.size - 44 ]       # Subchunk2Size: All remaining raw audio data

      output.seek(0)
      output.write(header.pack("A4L<A4A4L<S<S<L<L<S<S<A4L<"))
    end
  end
end
