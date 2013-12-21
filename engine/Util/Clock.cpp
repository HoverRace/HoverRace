
// Clock.cpp
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

#include "Clock.h"

namespace HoverRace {
namespace Util {

/**
 * Constructor.
 * The initial simulation time will be zero.
 */
Clock::Clock() :
	lastRead(),
	start(OS::Time()),
	offset(start)
{
}

/**
 * Constructor.
 * @param init The initial simulation time.
 */
Clock::Clock(const Duration &init) :
	lastRead(init),
	start(OS::Time()),
	offset(start, init)
{
}

void Clock::SetTime(const Duration &duration)
{
	lastRead = duration;
	start = Duration(OS::Time());
	offset = Duration(start, duration);
}

/**
 * Advance the clock to the current time.
 * @return The elapsed time since the last time the clock was advanced.
 */
Duration Clock::Advance()
{
	Duration prev = lastRead;
	lastRead = Duration(OS::Time(), offset);
	return Duration(lastRead, prev);
}

}  // namespace Util
}  // namespace HoverRace
