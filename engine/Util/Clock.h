
// Clock.h
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

#include "Duration.h"
#include "OS.h"

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
namespace Util {

/**
 * A game clock that manages the current time in the simulation.
 * @author Michael Imamura
 */
class MR_DllDeclare Clock
{
	public:
		Clock(OS::timestamp_t init=0);

	public:
		/**
		 * Retrieve the time of the last call to Advance().
		 */
		OS::timestamp_t GetTime() const { return lastRead; }
		void SetTime(OS::timestamp_t ts=0);

		OS::timestamp_t Advance();

	private:
		OS::timestamp_t lastRead;
		OS::timestamp_t start;
		OS::timestamp_t offset;
};

inline std::ostream &operator<<(std::ostream &os, const Clock &clock)
{
	Duration(clock.GetTime()).FmtLong(os);
	return os;
}

}  // namespace Util
}  // namespace HoverRace

#undef MR_DllDeclare
