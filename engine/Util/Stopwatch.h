
// Stopwatch.h
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

#include "Clock.h"
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
 * Records lap times.
 * @author Michael Imamura
 */
class MR_DllDeclare Stopwatch
{
	public:
		struct Lap
		{
			Lap(const std::string &name, const Duration &elapsed) :
				name(name), elapsed(elapsed) { }
			Lap(const Lap &o) = default;
			Lap(Lap &&o) = default;

			Lap &operator=(const Lap &o) = default;
			Lap &operator=(Lap &&lap) = default;

			std::string name;
			Duration elapsed;
		};

	public:
		Stopwatch(std::shared_ptr<Clock> clock);
		Stopwatch(std::shared_ptr<Clock> clock, const Duration &start);

	public:
		template<typename Fn>
		void ForEachLap(Fn fn)
		{
			std::for_each(laps.cbegin(), laps.cend(), fn);
		}

	public:
		Duration NextLap(const std::string &name);

	private:
		std::shared_ptr<Clock> clock;
		Duration lastLap;
		std::vector<Lap> laps;
};

}  // namespace Util
}  // namespace HoverRace

#undef MR_DllDeclare
