
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

#include "../Util/MR_Types.h"

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
	Color(MR_UInt8 a, MR_UInt8 r, MR_UInt8 g, MR_UInt8 b) :
		argb(((MR_UInt32)a << 24) + ((MR_UInt32)r << 16) + ((MR_UInt32)g << 8) + (MR_UInt32)b)
		{ }

	MR_UInt32 argb;
	struct
	{
		MR_UInt8 b, g, r, a;
	} bits;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
