
// Random.cpp
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

#include <thread>

#include "Log.h"

#include "Random.h"

namespace HoverRace {
namespace Util {

/**
 * Constructor.
 * Ensures that the PRNG is seeded with a random seed.
 */
RandSource::RandSource() :
	SUPER()
{
	HR_LOG(debug) << "Creating PRNG for thread: " << std::this_thread::get_id();

	seed(std::random_device()());
}

/**
 * Retrieve the per-thread random data source.
 *
 * If this is the first time this is called for a given thread, a new random
 * data source will be created and seeded.
 *
 * @return The random source.
 */
RandSource &RandSource::ForThread()
{
	thread_local RandSource source;
	return source;
}

}  // namespace Util
}  // namespace HoverRace
