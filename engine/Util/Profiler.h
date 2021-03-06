
// Profiler.h
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

#pragma once

#include <chrono>

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
 * Simple profiler.
 * @author Michael Imamura
 */
class MR_DllDeclare Profiler
{
public:
	Profiler() = delete;
	Profiler(const std::string &name);

public:
	using clock_t = std::chrono::high_resolution_clock;
	using dur_t = std::chrono::nanoseconds;

	class Sampler
	{
	public:
		Sampler() = delete;

		Sampler(Profiler &profiler) :
			profiler(profiler)
		{
			if ((++(profiler.sampling)) == 1) {
				profiler.sampleStart = clock_t::now();
			}
		}

		~Sampler()
		{
			if ((--(profiler.sampling)) == 0) {
				profiler.dur += (clock_t::now() - profiler.sampleStart);
			}
		}

		Sampler(const Sampler&) = delete;
		Sampler(Sampler&&) = default;

		Sampler &operator=(const Sampler&) = delete;
		Sampler &operator=(Sampler&&) = default;

	private:
		Profiler &profiler;
	};

	struct LapTime
	{
		LapTime() :
			time(dur_t::zero()),
			pctParent(std::numeric_limits<double>::quiet_NaN()) { }

		dur_t time;
		double pctParent;
	};

public:
	const std::string &GetName() const { return name; }
	dur_t GetDuration() const { return dur; }
	const LapTime &GetLastLap() const { return lap; }

	/**
	 * Retrieve the unaccounted-for time from the last call to Lap().
	 * @return The other time.
	 */
	const LapTime &GetOtherTime() const { return otherTime; }

	void Reset();

	const LapTime &Lap(const Profiler *parent = nullptr);

public:
	std::shared_ptr<Profiler> AddSub(const std::string &name);

private:
	dur_t dur;
	std::string name;
	std::vector<std::shared_ptr<Profiler>> subs;
	int sampling;
	clock_t::time_point sampleStart;
	LapTime lap;
	LapTime otherTime;
};

MR_DllDeclare inline std::ostream &operator<<(std::ostream &os,
	const Profiler &p)
{
	os << p.GetName() << ' ' << p.GetDuration().count();
	return os;
}

MR_DllDeclare inline std::ostream &operator<<(std::ostream &os,
	const Profiler::LapTime &lp)
{
	static boost::format fmt("%0.2f%% (%d)");
	os << fmt % lp.pctParent % lp.time.count();
	return os;
}

}  // namespace Util
}  // namespace HoverRace

#undef MR_DllDeclare
