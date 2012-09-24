#ifndef	RUBY_FFMPEG_UTIL_GD2_FORMAT_H
#define	RUBY_FFMPEG_UTIL_GD2_FORMAT_H

#include <stdint.h>

// Methods
int util_gd2_format_get_header_size();
uint8_t * util_gd2_format_write_header(uint8_t * buffer, int width, int height);

#endif // RUBY_FFMPEG_UTIL_GD2_FORMAT_H
