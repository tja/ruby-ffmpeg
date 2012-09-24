#ifndef	RUBY_FFMPEG_UTIL_BMP_FORMAT_H
#define	RUBY_FFMPEG_UTIL_BMP_FORMAT_H

#include <stdint.h>

// Methods
int util_bmp_format_get_header_size();
uint8_t * util_bmp_format_write_header(uint8_t * buffer, int width, int height);

#endif // RUBY_FFMPEG_UTIL_BMP_FORMAT_H
