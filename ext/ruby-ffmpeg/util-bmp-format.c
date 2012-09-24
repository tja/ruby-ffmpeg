#include "util-bmp-format.h"
#include "util-endian.h"

/*
**	Structures.
*/

#pragma pack(1)

// Bitmap File Header
struct BITMAPFILEHEADER {
	uint16_t					bfType;
	uint32_t					bfSize;
	uint16_t					bfReserved1;
	uint16_t					bfReserved2;
	uint32_t					bfOffBits;
};

// Bitmap Info Header
struct BITMAPINFOHEADER {
	uint32_t					biSize;
	int32_t						biWidth;
	int32_t						biHeight;
	uint16_t					biPlanes;
	uint16_t					biBitCount;
	uint32_t					biCompression;
	uint32_t					biSizeImage;
	int32_t						biXPelsPerMeter;
	int32_t						biYPelsPerMeter;
	uint32_t					biClrUsed;
	uint32_t					biClrImportant;
};

// Full File
struct BITMAPFILE {
	struct BITMAPFILEHEADER		bfh;
	struct BITMAPINFOHEADER		bih;
};


/*
**	Methods.
*/
	
// Return size of BMP header
int util_bmp_format_get_header_size() {
	return sizeof(struct BITMAPFILE);
}

// Fill in BMP header data
uint8_t * util_bmp_format_write_header(uint8_t * buffer, int width, int height) {
	// BITMAPFILEHEADER
	((struct BITMAPFILE *)buffer)->bfh.bfType			= HTOLS('B' | ('M' << 8));
	((struct BITMAPFILE *)buffer)->bfh.bfSize			= HTOLL(sizeof(struct BITMAPFILE) + width * height * 4);
	((struct BITMAPFILE *)buffer)->bfh.bfReserved1		= HTOLL(0);
	((struct BITMAPFILE *)buffer)->bfh.bfReserved2		= HTOLL(0);
	((struct BITMAPFILE *)buffer)->bfh.bfOffBits		= HTOLL(sizeof(struct BITMAPFILE));

	// BITMAPINFOHEADER
	((struct BITMAPFILE *)buffer)->bih.biSize			= HTOLL(sizeof(struct BITMAPINFOHEADER));
	((struct BITMAPFILE *)buffer)->bih.biWidth			= HTOLL(width);
	((struct BITMAPFILE *)buffer)->bih.biHeight			= HTOLL(-height);	// Vertical Flip
	((struct BITMAPFILE *)buffer)->bih.biPlanes			= HTOLS(1);
	((struct BITMAPFILE *)buffer)->bih.biBitCount		= HTOLS(32);
	((struct BITMAPFILE *)buffer)->bih.biCompression	= HTOLL(/* BI_RGB */ 0);
	((struct BITMAPFILE *)buffer)->bih.biSizeImage		= HTOLL(width * height * 4);
    ((struct BITMAPFILE *)buffer)->bih.biXPelsPerMeter	= HTOLL(2835);
    ((struct BITMAPFILE *)buffer)->bih.biYPelsPerMeter	= HTOLL(2835);
    ((struct BITMAPFILE *)buffer)->bih.biClrUsed		= HTOLL(0);
    ((struct BITMAPFILE *)buffer)->bih.biClrImportant	= HTOLL(0);

	// Return pointer right after structure
	return (uint8_t *)&((struct BITMAPFILE *)buffer)[1];
}
