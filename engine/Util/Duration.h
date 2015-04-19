
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

#include <boost/operators.hpp>

#include "MR_Types.h"
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
 * The relative time between two timestamps.
 * Unlike OS::timestamp_t, this can represent negative times.
 * @author Michael Imamura
 */
class MR_DllDeclare Duration :
	private boost::totally_ordered<Duration, Duration>,
	private boost::totally_ordered<Duration, MR_Int64>
{
public:
	using dur_t = MR_Int64;

	/**
	 * Constructor from two timestamps.
	 * @param later The later of the two timestamps.
	 * @param earlier The earlier of the two timestamps.
	 */
	Duration(OS::timestamp_t later, OS::timestamp_t earlier) :
		duration(OS::TimeDiff(later, earlier)) { }

	/**
	 * Constructor from two durations.
	 * @param later The later of the two durations.
	 * @param earlier The earlier of the two durations.
	 */
	Duration(const Duration &later, const Duration &earlier) :
		duration(later.duration - earlier.duration) { }

	/**
	 * Constructor.
	 * @param duration The time in milliseconds.
	 */
	Duration(dur_t duration=0) : duration(duration) { }

	Duration(const std::string &s) : Duration(ParseDuration(s)) { }

	// Fix ambiguity between char* and dur_t.
	// See specialization for T=char below.
	template<class T>
	Duration(const T *s);

	Duration(const Duration&) = default;
	Duration(Duration&&) = default;

	Duration &operator=(const Duration&) = default;
	Duration &operator=(Duration&&) = default;

	Duration operator-() const
	{
		return Duration(-duration);
	}

	/**
	 * Convert the duration to milliseconds.
	 * @return This duration.
	 */
	dur_t ToMs() const { return duration; }

private:
	static dur_t ParseDuration(const std::string &s);

public:
	std::ostream &FmtLong(std::ostream &os) const;
	std::string FmtLong() const;
	std::ostream &FmtShort(std::ostream &os) const;
	std::string FmtShort() const;

private:
	dur_t duration;

	friend bool operator==(const Duration &a, const Duration &b);
	friend bool operator==(const Duration &duration, Duration::dur_t ts);
	friend bool operator==(Duration::dur_t ts, const Duration &duration);
	friend bool operator<(const Duration &a, const Duration &b);
	friend bool operator<(const Duration &duration, Duration::dur_t ts);
	friend bool operator<(Duration::dur_t ts, const Duration &duration);
};

template<>
inline Duration::Duration(const char *s) : Duration(std::string(s)) { }

inline std::ostream &operator<<(std::ostream &os, const Duration &dur)
{
	return dur.FmtShort(os);
}

inline bool operator==(const Duration &a, const Duration &b)
{
	return a.duration == b.duration;
}

inline bool operator==(const Duration &a, Duration::dur_t b)
{
	return a.duration == b;
}

inline bool operator==(Duration::dur_t a, const Duration &b)
{
	return a == b.duration;
}

inline bool operator<(const Duration &a, const Duration &b)
{
	return a.duration < b.duration;
}

inline bool operator<(const Duration &a, Duration::dur_t b)
{
	return a.duration < b;
}

inline bool operator<(Duration::dur_t a, const Duration &b)
{
	return a < b.duration;
}

}  // namespace Util
}  // namespace HoverRace

#undef MR_DllDeclare
