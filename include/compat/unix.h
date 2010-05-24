
/* Compatibility for Win32-specific defines. */

#include <assert.h>
#include <inttypes.h>

#define ASSERT(e) assert(e)

#define BOOL int
#define DWORD unsigned int
#define TRUE 1
#define FALSE 0

#define _snprintf snprintf

#define Int32x32To64(x,y) \
	static_cast<int64_t>( \
		static_cast<int64_t>(x) * static_cast<int64_t>(y));

