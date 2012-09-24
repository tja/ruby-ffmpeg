#include "util-gd2-format.h"
#include "util-endian.h"

/*
**	Structures.
*/

#pragma pack(1)

// Bitmap File Header
struct GD2FILE {
	char		magic[4];
	uint16_t	version;
	uint16_t	width;
	uint16_t	height;
	uint16_t	chunk_size;
	uint16_t	format;
	uint16_t	chunk_count_x;
	uint16_t	chunk_count_y;
	uint8_t		truecolor;
	int32_t		transparent;
};


/*
**	Methods.
*/
	
// Return size of GD2 header
int util_gd2_format_get_header_size() {
	return sizeof(struct GD2FILE);
}

// Fill in GD2 header data
uint8_t * util_gd2_format_write_header(uint8_t * buffer, int width, int height) {
	// GD2FILE
	((struct GD2FILE *)buffer)->magic[0]		= 'g';
	((struct GD2FILE *)buffer)->magic[1]		= 'd';
	((struct GD2FILE *)buffer)->magic[2]		= '2';
	((struct GD2FILE *)buffer)->magic[3]		= '\0';
	((struct GD2FILE *)buffer)->version			= HTOBS(2);
	((struct GD2FILE *)buffer)->width			= HTOBS(width);
	((struct GD2FILE *)buffer)->height			= HTOBS(height);
	((struct GD2FILE *)buffer)->chunk_size		= HTOBS(4096);
	((struct GD2FILE *)buffer)->format			= HTOBS(/* GD2_FMT_TRUECOLOR_RAW */ 3);
	((struct GD2FILE *)buffer)->chunk_count_x	= HTOBS(1);
	((struct GD2FILE *)buffer)->chunk_count_y	= HTOBS(1);
	((struct GD2FILE *)buffer)->truecolor		= 1;
	((struct GD2FILE *)buffer)->transparent		= HTOBL(-1);

	// Return pointer right after structure
	return (uint8_t *)&((struct GD2FILE *)buffer)[1];
}
