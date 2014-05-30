
// Stopwatch.cpp
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

#include "StdAfx.h"

#include "Log.h"

#include "Stopwatch.h"

namespace HoverRace {
namespace Util {

namespace {

	std::shared_ptr<Clock> &CheckedClock(std::shared_ptr<Clock> &clock)
	{
		if (!clock) {
			Log::Warn("Stopwatch created with null clock (using default)");
			clock.reset(new Clock());
		}
		return clock;
	}

}

/**
 * Constructor.
 * The start time of the first lap is recorded as the current time of the
 * clock.
 * @param clock The clock to use for timing.
 */
Stopwatch::Stopwatch(std::shared_ptr<Clock> clock) :
	clock(std::move(CheckedClock(clock)))
{
	lastLap = this->clock->GetTime();
}

/**
 * Constructor.
 * @param clock The clock to use for timing.
 * @param start The timestamp of the start of the first lap.
 */
Stopwatch::Stopwatch(std::shared_ptr<Clock> clock, const Duration &start) :
	clock(std::move(CheckedClock(clock))), lastLap(start)
{
}

/**
 * Start a new lap.
 * @param name The name of the lap that just ended.
 * @return The elapsed time for the lap that just ended.
 */
Duration Stopwatch::NextLap(const std::string &name)
{
	Duration ts(clock->GetTime());
	Duration elapsed(ts, lastLap);
	lastLap = ts;

	laps.emplace_back(name, Duration(ts, lastLap));

	return elapsed;
}

}  // namespace Util
}  // namespace HoverRace

