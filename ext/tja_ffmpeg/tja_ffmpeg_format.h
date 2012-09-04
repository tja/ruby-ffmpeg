#ifndef	FFMPEG_FORMAT_H
#define	FFMPEG_FORMAT_H

#include <ruby.h>

// helper
char const * format_version_string();

// entry points
VALUE format_alloc(VALUE klass);
VALUE format_initialize(VALUE self, VALUE stream);

#endif // FFMPEG_FORMAT_H