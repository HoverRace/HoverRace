
/* Compatibility for Win32-specific defines. */

#include <assert.h>
#include <inttypes.h>

#define ASSERT(e) assert(e)

#define BOOL int
#define DWORD unsigned int
#define TRUE 1
#define FALSE 0

struct GUID
{
	uint32_t Data1;
	uint16_t Data2;
	uint16_t Data3;
	uint8_t Data4[8];
};

#define _snprintf snprintf

#define Int32x32To64(x,y) \
	static_cast<int64_t>( \
		static_cast<int64_t>(x) * static_cast<int64_t>(y))

#define Int64ShraMod32(x,y) \
	(static_cast<int64_t>(x) >> (y))

#define MulDiv(x,y,z) \
	static_cast<int32_t>( \
		(static_cast<int64_t>(x) * static_cast<int64_t>(y)) / (z))

