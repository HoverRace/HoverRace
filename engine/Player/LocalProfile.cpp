
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

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "../Util/Config.h"
#include "../Util/Str.h"
#include "../Util/yaml/Emitter.h"
#include "../Util/yaml/MapNode.h"
#include "../Util/yaml/ScalarNode.h"
#include "../Util/yaml/SeqNode.h"
#include "../Util/yaml/Parser.h"
#include "ProfileExn.h"

#include "LocalProfile.h"

namespace fs = boost::filesystem;
using namespace HoverRace::Util;

#define EMIT_VAR(emitter, name) \
	(emitter).MapKey(#name); \
	(emitter).Value(name);

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
 * @throw ProfileExn The profile could not be loaded.
 */
LocalProfile::LocalProfile(const boost::uuids::uuid &uid) :
	SUPER(uid), loaded(false)
{
	const auto *cfg = Config::GetInstance();
	auto path = cfg->GetProfilePath(boost::uuids::to_string(uid));

	if (!fs::is_directory(path)) {
		throw ProfileExn(
			"Profile path does not exist or is not a directory: " +
			(const std::string&)Str::PU(path));
	}

	//TODO
}

void LocalProfile::Save()
{
	const auto *cfg = Config::GetInstance();
	auto path = cfg->GetProfilePath(boost::uuids::to_string(GetUid()));

	if (!fs::exists(path)) {
		if (!fs::create_directories(path)) {
			throw ProfileExn("Unable to create profile directory: " +
				(const std::string&)Str::PU(path));
		}
	}

	path /= "profile.yml";

	try {
		fs::ofstream out{ path };
		yaml::Emitter emitter{ out };

		emitter.StartMap();

		emitter.MapKey("uid");
		emitter.Value(boost::uuids::to_string(GetUid()));

		emitter.MapKey("name");
		emitter.Value(GetName());

		emitter.MapKey("colors");
		emitter.StartSeq();
		emitter.Value(boost::lexical_cast<std::string>(GetPrimaryColor()));
		emitter.Value(boost::lexical_cast<std::string>(GetSecondaryColor()));
		emitter.EndSeq();

		emitter.EndMap();
	}
	catch (yaml::EmitterExn &ex) {
		throw std::exception(ex);
	}
}

}  // namespace Player
}  // namespace HoverRace
