
// SelFmt.h
//
// Copyright (c) 2013, 2014 Michael Imamura.
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

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
namespace Util {

extern MR_DllDeclare const int SEL_FMT_INDEX;

enum {
	SEL_FMT_DEFAULT,
	SEL_FMT_PANGO,
};

template<int FMT>
std::ostream &SelFmt(std::ostream &os)
{
	os.iword(SEL_FMT_INDEX) = FMT;
	return os;
}

inline long GetSelFmt(std::ostream &os)
{
	return os.iword(SEL_FMT_INDEX);
}

}  // namespace Util
}  // namespace HoverRace
