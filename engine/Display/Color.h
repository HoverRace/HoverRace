
// Color.h
//
// Copyright (c) 2013 Michael Imamura.
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

#pragma once

#include <boost/format.hpp>

#include "../Util/MR_Types.h"
#include "../Util/SelFmt.h"

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
namespace Display {

union MR_DllDeclare Color
{
	Color() : argb(0) { }
	Color(MR_UInt32 argb) : argb(argb) { }
	Color(MR_UInt8 a, MR_UInt8 r, MR_UInt8 g, MR_UInt8 b) :
		argb(((MR_UInt32)a << 24) + ((MR_UInt32)r << 16) + ((MR_UInt32)g << 8) + (MR_UInt32)b)
		{ }

	Color &operator=(MR_UInt32 argb)
	{
		this->argb = argb;
		return *this;
	}

	MR_UInt32 argb;
	struct
	{
		MR_UInt8 b, g, r, a;
	} bits;
};

extern MR_DllDeclare const Color COLOR_WHITE;
extern MR_DllDeclare const Color COLOR_BLACK;

MR_DllDeclare inline bool operator==(const Color &a, const Color &b)
{
	return a.argb == b.argb;
}

MR_DllDeclare inline bool operator!=(const Color &a, const Color &b)
{
	return !operator==(a, b);
}

MR_DllDeclare inline std::ostream &operator<<(std::ostream &os, const Color &c)
{
	os << "0x" << boost::format("%08x") % c.argb;
	return os;
}

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
