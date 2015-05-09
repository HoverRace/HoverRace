
// Profiler.cpp
//
// Copyright (c) 2015 Michael Imamura.
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

#include "Profiler.h"

namespace HoverRace {
namespace Util {

Profiler::Profiler(const std::string &name) :
	dur(dur_t::zero()), name(name), sampling(0)
{
}

/**
 * Resets the accumulated time.
 *
 * All subsets are also reset.
 */
void Profiler::Reset()
{
	dur = dur_t::zero();

	for (auto &ent : subs) {
		ent->Reset();
	}
}

/**
 * Calculate the statistics for the current period, then reset the counters
 * to start the next period.
 * @param parent The parent profiler (@c nullptr if this is the root profiler).
 * @return The lap statistics (also available via GetLastLap()).
 */
const Profiler::LapTime &Profiler::Lap(const Profiler *parent)
{
	lap.time = dur;
	if (parent) {
		auto parentDur = parent->GetDuration().count();
		if (parentDur == 0) {
			lap.pctParent = 0;
		}
		else {
			lap.pctParent =
				static_cast<double>(dur.count() * 100) /
				static_cast<double>(parentDur);
		}
	}
	else {
		lap.pctParent = 100;
	}

	for (auto &ent : subs) {
		ent->Lap(this);
	}

	dur = dur_t::zero();

	return lap;
}

/**
 * Add a new subset.
 * @param name The name of the subset.
 * @return The created subset (never @c nullptr).
 */
std::shared_ptr<Profiler> Profiler::AddSub(const std::string &name)
{
	subs.emplace_back(std::make_shared<Profiler>(name));
	return subs.back();
}

}  // namespace Util
}  // namespace HoverRace
