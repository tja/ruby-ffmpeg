#ifndef	RUBY_FFMPEG_UTIL_ENDIAN_H
#define	RUBY_FFMPEG_UTIL_ENDIAN_H

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#	define	HTOLS(X) 	(X)
#	define	HTOLL(X) 	(X)
#	define	HTOBS(X) 	((((X) & 0x00ff) << 8) | (((X) & 0xff00) >> 8))
#	define	HTOBL(X) 	((((X) & 0x000000ff) << 24) | (((X) & 0x0000ff00) << 8) | (((X) & 0x00ff0000) >> 8) | (((X) & 0xff000000) >> 24))
#else
#	define	HTOLS(X) 	((((X) & 0x00ff) << 8) | (((X) & 0xff00) >> 8))
#	define	HTOLL(X) 	((((X) & 0x000000ff) << 24) | (((X) & 0x0000ff00) << 8) | (((X) & 0x00ff0000) >> 8) | (((X) & 0xff000000) >> 24))
#	define	HTOBS(X) 	(X)
#	define	HTOBL(X) 	(X)
#endif

#endif // RUBY_FFMPEG_UTIL_ENDIAN_H
