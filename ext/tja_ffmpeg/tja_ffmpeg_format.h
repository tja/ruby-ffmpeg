#ifndef	FFMPEG_FORMAT_H
#define	FFMPEG_FORMAT_H

#include <ruby.h>

// ...
extern VALUE format_klass;

// entry points
VALUE format_alloc(VALUE klass);
VALUE format_initialize(VALUE self, VALUE stream);

// properties
VALUE format_name(VALUE self);
VALUE format_description(VALUE self);
VALUE format_streams(VALUE self);

// helper
char const * format_version_string();

#endif // FFMPEG_FORMAT_H