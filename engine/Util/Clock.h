
// Clock.h
//
// Copyright (c) 2013-2015 Michael Imamura.
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

/**
 * A game clock that manages the current time in the simulation.
 * @author Michael Imamura
 */
class MR_DllDeclare Clock : public std::enable_shared_from_this<Clock>
{
public:
	Clock();
	Clock(const Duration &init);

	// We can't copy the alarm signals, so we can't be copyable either.
	Clock(const Clock&) = delete;
	Clock(Clock&&) = default;
	Clock &operator=(const Clock&) = delete;
	Clock &operator=(Clock&&) = default;

	virtual ~Clock() { }

public:
	std::string FmtLong() const { return lastRead.FmtLong(); }
	std::string FmtShort() const { return lastRead.FmtShort(); }

public:
	/**
	 * Retrieve the time of the last call to Advance().
	 */
	const Duration &GetTime() const { return lastRead; }
	void SetTime(const Duration &duration=Duration());

	Duration Advance();

public:
	using alarmSignal_t =  boost::signals2::signal<void()>;

	/**
	 * Remove all alarms.
	 */
	void ClearAlarms()
	{
		alarms.clear();
	}

	/**
	 * Set a one-shot alarm to run at a specific time.
	 *
	 * It's not guaranteed that the alarm will be fired at the exact time
	 * since alarms are only fired when Advance() is called; however,
	 * an alarm will never be fired before it's timestamp.
	 *
	 * @param duration The clock timestamp.
	 * @param fn The function to invoke.
	 * @return The signal connection; use this to cancel the alarm by
	 *         disconnecting it.
	 */
	template<class Fn>
	boost::signals2::connection At(const Duration &duration, Fn &&fn)
	{
		auto &sig = alarms[duration];
		if (!sig) {
			sig.reset(new alarmSignal_t());
		}
		return sig->connect(std::forward<Fn>(fn));
	}

private:
	Duration lastRead;
	Duration start;
	Duration offset;

	// Sorted list of pending alarms.
	// We need to wrap the signal in a unique_ptr because depending on the
	// Boost version, it may not have move semantics.
	std::map<Duration, std::unique_ptr<alarmSignal_t>> alarms;
};

inline std::ostream &operator<<(std::ostream &os, const Clock &clock)
{
	clock.GetTime().FmtLong(os);
	return os;
}

}  // namespace Util
}  // namespace HoverRace

#undef MR_DllDeclare
