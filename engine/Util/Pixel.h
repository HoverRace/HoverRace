// Pixel.h
// Useful pixel struct

#ifndef PIXEL_H
#define PIXEL_H

#include "MR_Types.h"

// 24-bit pixel
#ifdef _MSC_VER
#	pragma pack(push)
#	pragma pack(1) /* I need this struct to be 3 bytes long */
#endif

struct Pixel24 {
	MR_UInt8 r;
	MR_UInt8 g;
	MR_UInt8 b;
}
#ifdef __GNUC__
	__attribute__((packed))
#endif
;

#ifdef _MSC_VER
#	pragma pack(pop)
#endif

#endif
