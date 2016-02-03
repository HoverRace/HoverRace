
// Color.cpp
//
// Copyright (c) 2016 Michael Imamura.
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

#include <boost/format.hpp>

#include "../Util/Log.h"

#include "Color.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

std::istream &operator>>(std::istream &is, Color &c)
{
	if (!is) return is;

	std::string ris;
	ris.reserve(16);
	is.width(static_cast<std::streamsize>(ris.capacity()));
	is >> ris;

	if (ris.empty()) return is;

	const char *start = ris.c_str();
	char *end;

	// Skip initial "#".
	if (*start == '#') ++start;

	auto val = strtoul(start, &end, 16);
	if (start == end || errno == ERANGE) {
		HR_LOG(warning) << "Invalid color: " << ris;
	}
	else {
		c.argb = static_cast<MR_UInt32>(val);
	}

	return is;
}

std::ostream &operator<<(std::ostream &os, const Color &c)
{
	static boost::format FMT{ "%08x" };

	os << '#' << FMT % c.argb;
	return os;
}

}  // namespace Display
}  // namespace HoverRace
