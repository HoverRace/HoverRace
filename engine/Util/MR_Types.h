// MR_Types.h // Some usefull known size integer
//
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
//
// Licensed under GrokkSoft HoverRace SourceCode License v1.0(the "License");
// you may not use this file except in compliance with the License.
//
// A copy of the license should have been attached to the package from which
// you have taken this file. If you can not find the license you can not use
// this file.
//
//
// The author makes no representations about the suitability of
// this software for any purpose.  It is provided "as is" "AS IS",
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.
//
// See the License for the specific language governing permissions
// and limitations under the License.
//

#ifndef MR_TYPES_H
#define MR_TYPES_H

#ifdef _WIN32
#	include <windows.h>
#	include <winnt.h>

	typedef signed char MR_Int8;
	typedef unsigned char MR_UInt8;
	typedef signed short MR_Int16;
	typedef unsigned short MR_UInt16;
	typedef signed int MR_Int32;
	typedef unsigned int MR_UInt32;
	typedef LONGLONG MR_Int64;
	typedef DWORDLONG MR_UInt64;

#else
#  include <inttypes.h>
	
	typedef int8_t MR_Int8;
	typedef uint8_t MR_UInt8;
	typedef int16_t MR_Int16;
	typedef uint16_t MR_UInt16;
	typedef int32_t MR_Int32;
	typedef uint32_t MR_UInt32;
	typedef int64_t MR_Int64;
	typedef uint64_t MR_UInt64;

#endif

#endif
