
// Duration.cpp
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

#include "../StdAfx.h"

#include <utf8/utf8.h>

#include "../Exception.h"

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

	bool next = ts > 0;
	if (next) {
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

/**
 * Parse a duration from a string.
 *
 * Example strings:
 * - "300" or "300ms" - 300 milliseconds.
 * - "4s" - 4 seconds.
 * - "12m" - 12 minutes.
 *
 * @param s The duration.
 * @return The parsed duration.
 * @throws Exception The duration could not be parsed.
 */
Duration::dur_t Duration::ParseDuration(const std::string &s)
{
	dur_t retv = 0;
	std::string suffix;

	bool expectDigit = true;

	for (auto iter = s.cbegin(), iend = s.cend(); iter != iend;) {
		MR_UInt32 ch = utf8::next(iter, iend);

		if (expectDigit && (ch >= '0' && ch <= '9')) {
			retv = retv * 10 + (ch - '0');
		}
		else if (ch >= 'a' && ch <= 'z') {
			expectDigit = false;
			suffix += static_cast<char>(ch);
		}
		else {
			std::stringstream oss;
			oss << "Invalid duration: " << s;
			throw Exception(oss.str());
		}
	}

	if (!suffix.empty()) {
		if (suffix == "ms") {
			// Do nothing, this is the default.
		}
		else if (suffix == "s") {
			retv *= 1000;
		}
		else if (suffix == "m") {
			retv *= 60 * 1000;
		}
		else if (suffix == "h") {
			retv *= 60 * 60 * 1000;
		}
		else {
			std::stringstream oss;
			oss << "Invalid duration '" << s << "': unknown suffix: " << suffix;
			throw Exception(oss.str());
		}
	}

	return retv;
}

}  // namespace Util
}  // namespace HoverRace
