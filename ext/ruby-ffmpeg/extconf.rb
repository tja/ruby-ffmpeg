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

$CFLAGS << ' -Wall '

create_makefile('ruby-ffmpeg/ruby_ffmpeg_ext')