
// MR_Types.h
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

#pragma once

#ifdef _WIN32

	typedef signed __int8 MR_Int8;
	typedef unsigned __int8 MR_UInt8;
	typedef signed __int16 MR_Int16;
	typedef unsigned __int16 MR_UInt16;
	typedef signed __int32 MR_Int32;
	typedef unsigned __int32 MR_UInt32;
	typedef signed __int64 MR_Int64;
	typedef unsigned __int64 MR_UInt64;

#else
#	include <inttypes.h>
	
	typedef int8_t MR_Int8;
	typedef uint8_t MR_UInt8;
	typedef int16_t MR_Int16;
	typedef uint16_t MR_UInt16;
	typedef int32_t MR_Int32;
	typedef uint32_t MR_UInt32;
	typedef int64_t MR_Int64;
	typedef uint64_t MR_UInt64;

#endif
