
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
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "../Util/Config.h"
#include "../Util/Log.h"
#include "../Util/Str.h"
#include "../Util/yaml/Emitter.h"
#include "../Util/yaml/MapNode.h"
#include "../Util/yaml/ScalarNode.h"
#include "../Util/yaml/SeqNode.h"
#include "../Util/yaml/Parser.h"
#include "ProfileExn.h"

#include "LocalProfile.h"

namespace fs = boost::filesystem;
namespace uuid = boost::uuids;
using namespace HoverRace::Util;

namespace HoverRace {
namespace Player {

namespace {

/**
 * Attempt to read a color from a YAML node.
 */
boost::optional<Display::Color> ReadColor(yaml::Node *node)
{
	if (!node) return boost::none;

	if (auto val = dynamic_cast<yaml::ScalarNode*>(node)) {
		auto str = val->AsString();
		return boost::lexical_cast<Display::Color>(val->AsString());
	}
	else {
		HR_LOG(warning) << "Expected scalar in sequence.";
	}

	return boost::none;
}

}  // namespace


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
	auto path = cfg->GetProfilePath(uuid::to_string(uid));
	path /= "profile.yml";

	const std::string pathStr = (const std::string&)Str::PU(path);

	if (!fs::exists(path)) {
		throw ProfileExn(
			"Profile does not exist: " + pathStr);
	}

	fs::ifstream in{ path };
	if (!in.is_open()) {
		throw ProfileExn(
			"Could not open profile: " + pathStr);
	}

	try {
		yaml::Parser parser{ in };
		auto node = parser.GetRootNode();

		if (auto root = dynamic_cast<yaml::MapNode*>(node)) {
			Load(root, pathStr);
		}
	}
	catch (yaml::EmptyDocParserExn&) {
		// Ignore.
	}
	catch (yaml::ParserExn &ex) {
		throw ProfileExn(ex.what());
	}

	loaded = true;
}

void LocalProfile::Load(yaml::MapNode *root, const std::string &filename)
{
	std::string s;
	uuid::uuid readUid;

	// Verify that the UID matches.
	root->ReadString("uid", s);
	try {
		readUid = uuid::string_generator()(s);
	}
	catch (...) {
		throw ProfileExn("Invalid UID in profile: " + filename);
	}
	if (readUid != GetUid()) {
		throw ProfileExn("Profile UID (" + uuid::to_string(readUid) + ") "
			"does not match expected UID (" + uuid::to_string(GetUid()) + ")");
	}

	s = GetName();
	root->ReadString("name", s);
	SetName(s);

	auto colorNode = root->Get("colors");
	if (auto colorSeq = dynamic_cast<yaml::SeqNode*>(colorNode)) {
		auto iter = colorSeq->begin();
		auto endIter = colorSeq->end();
		if (iter != endIter) {
			if (auto color = ReadColor(*iter)) {
				SetPrimaryColor(*color);
			}
			++iter;
		}
		if (iter != endIter) {
			if (auto color = ReadColor(*iter)) {
				SetSecondaryColor(*color);
			}
			++iter;
		}
	}
	else {
		HR_LOG(warning) << "\"colors\" is not a sequence: " << filename;
	}
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

	fs::ofstream out{ path };
	if (!out.is_open()) {
		throw ProfileExn("Unable to create profile file: " +
			(const std::string&)Str::PU(path));
	}

	try {
		yaml::Emitter emitter{ out };

		emitter.StartMap();

		emitter.MapKey("uid");
		emitter.Value(boost::uuids::to_string(GetUid()));

		emitter.MapKey("name");
		emitter.Value(GetName());

		emitter.MapKey("colors");
		emitter.StartSeq();
		emitter.Value(GetPrimaryColor());
		emitter.Value(GetSecondaryColor());
		emitter.EndSeq();

		emitter.EndMap();
	}
	catch (yaml::EmitterExn &ex) {
		throw std::exception(ex);
	}
}

}  // namespace Player
}  // namespace HoverRace
