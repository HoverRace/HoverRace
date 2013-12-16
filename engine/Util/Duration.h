
// Duration.h
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

#include <iomanip>

#include "MR_Types.h"
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
 * The relative time between two timestamps.
 * Unlike OS::timestamp_t, this can represent negative times.
 * @author Michael Imamura
 */
class MR_DllDeclare Duration
{
	typedef MR_Int64 dur_t;
	public:
		/**
		 * Constructor.
		 * @param later The later of the two timestamps.
		 * @param earlier The earlier of the two timestamps.
		 */
		Duration(OS::timestamp_t later, OS::timestamp_t earlier) :
			duration(later >= earlier ?
				static_cast<dur_t>(OS::TimeDiff(later, earlier)) :
				-static_cast<dur_t>(OS::TimeDiff(earlier, later))) { }

		/**
		 * Constructor.
		 * @param duration The time in milliseconds.
		 */
		Duration(dur_t duration) : duration(duration) { }

	public:
		std::ostream &Duration::FmtLong(std::ostream &os) const;
		std::string FmtLong() const;
		std::ostream &FmtShort(std::ostream &os) const;
		std::string FmtShort() const;

	private:
		dur_t duration;
};

inline std::ostream &operator<<(std::ostream &os, const Duration &dur)
{
	return dur.FmtShort(os);
}

}  // namespace Util
}  // namespace HoverRace

#undef MR_DllDeclare
