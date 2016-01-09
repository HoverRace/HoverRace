
// LocalProfile.cpp
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

#include <boost/uuid/random_generator.hpp>

#include "../Util/yaml/Emitter.h"
#include "../Util/yaml/MapNode.h"
#include "../Util/yaml/ScalarNode.h"
#include "../Util/yaml/SeqNode.h"
#include "../Util/yaml/Parser.h"
#include "ProfileLoadExn.h"

#include "LocalProfile.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Player {

/**
 * Constructor for a new local profile.
 *
 * The new profile will be assigned a new unique UID.
 */
LocalProfile::LocalProfile() :
	SUPER(boost::uuids::random_generator{}())
{
}

/**
 * Load a profile from the local filesystem.
 * @throw ProfileLoadExn The profile could not be loaded.
 */
LocalProfile::LocalProfile(const boost::uuids::uuid &uid) :
	SUPER(uid), loaded(false)
{
	//TODO
}

}  // namespace Player
}  // namespace HoverRace
