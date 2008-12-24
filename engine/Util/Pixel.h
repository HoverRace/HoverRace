// Pixel.h
// Useful pixel struct

#ifndef PIXEL_H
#define PIXEL_H

#include "MR_Types.h"

// 24-bit pixel
#pragma pack(push)
#pragma pack(1) /* I need this struct to be 3 bytes long */

struct Pixel24 {
	MR_UInt8 r;
	MR_UInt8 g;
	MR_UInt8 b;
};

#pragma pack(pop)

#endif