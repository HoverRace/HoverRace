
// Duration.cpp
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

#include "Duration.h"

namespace HoverRace {
namespace Util {

/**
 * Stream out the full-length format.
 *
 * The format is: <em>hours</em><tt>:</tt><em>minutes</em><tt>:</tt><em>seconds</em><tt>.</tt><em>milliseconds</em>
 *
 * @param os The output stream.
 * @return The same output stream.
 */
std::ostream &Duration::FmtLong(std::ostream &os) const
{
	auto ts = (duration < 0) ? -duration : duration;
	auto ms = ts % 1000;
	auto secs = (ts /= 1000) % 60;
	auto mins = (ts /= 60) % 60;
	ts /= 60;

	auto oldFill = os.fill('0');

	if (duration < 0) {
		os << '-';
	}

	os << ts << ':' <<
		std::setw(2) << mins << ':' <<
		std::setw(2) << secs <<
		std::use_facet<std::numpunct<char>>(OS::locale).decimal_point() <<
		std::setw(3) << ms;

	os.fill(oldFill);

	return os;
}

/**
 * Convert to string using the full-length format.
 *
 * The format is: <em>hours</em><tt>:</tt><em>minutes</em><tt>:</tt><em>seconds</em><tt>.</tt><em>milliseconds</em>
 *
 * @return The formatted string.
 */
std::string Duration::FmtLong() const
{
	std::ostringstream oss;
	FmtLong(oss);
	return oss.str();
}

/**
 * Stream out the short format.
 *
 * The format is the same as the long format, but leading zero components
 * are ignored.
 *
 * @param os The output stream.
 * @return The same output stream.
 */
std::ostream &Duration::FmtShort(std::ostream &os) const
{
	auto ts = (duration < 0) ? -duration : duration;
	auto ms = ts % 1000;
	auto secs = (ts /= 1000) % 60;
	auto mins = (ts /= 60) % 60;
	ts /= 60;

	auto oldFill = os.fill('0');

	if (duration < 0) {
		os << '-';
	}

	bool next;
	if ((next = ts > 0)) {
		os << ts << ':' << std::setw(2);
	}
	if (next || mins > 0) {
		os << mins << ':' << std::setw(2);
	}
	os << secs << 
		std::use_facet<std::numpunct<char>>(OS::locale).decimal_point() <<
		std::setw(3) << ms;

	os.fill(oldFill);

	return os;
}

/**
 * Convert to string using the short format.
 *
 * The format is the same as the long format, but leading zero components
 * are ignored.
 *
 * @return The formatted string.
 */
std::string Duration::FmtShort() const
{
	std::ostringstream oss;
	FmtShort(oss);
	return oss.str();
}

}  // namespace Util
}  // namespace HoverRace
