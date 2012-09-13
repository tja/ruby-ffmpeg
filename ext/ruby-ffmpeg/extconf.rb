require 'mkmf'

LIB_DIRS = [
  RbConfig::CONFIG['libdir'],
  '/opt/local/lib',
  '/usr/local/lib',
  '/usr/lib'
]
  
INCLUDE_DIRS = [
  RbConfig::CONFIG['includedir'],
  '/opt/local/include',
  '/usr/local/include',
  '/usr/include'
]

dir_config('', INCLUDE_DIRS, LIB_DIRS)

have_library('avformat') or raise
have_library('avcodec') or raise
have_library('avutil') or raise
have_library('swscale') or raise

$CFLAGS << " -Wall "                    # Show all warnings
$CFLAGS << " -Wno-shorten-64-to-32 "    # Except 64-to-32 shortenings, as they are mostly caused by FFMPEG

create_makefile('ruby-ffmpeg/ruby_ffmpeg_ext')