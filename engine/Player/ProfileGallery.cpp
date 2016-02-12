
// ProfileGallery.cpp
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
#include <boost/uuid/string_generator.hpp>

#include "../Util/Config.h"
#include "../Util/Log.h"
#include "../Util/Str.h"
#include "LocalProfile.h"
#include "ProfileExn.h"

#include "ProfileGallery.h"

namespace fs = boost::filesystem;
namespace uuid = boost::uuids;
using namespace HoverRace::Util;

namespace HoverRace {
namespace Player {

/**
 * Constructor.
 * @param path The base profile directory (may be empty to use default).
 * @throw ProfileExn The full list of profiles could not be loaded.
 */
ProfileGallery::ProfileGallery(const OS::path_t &path) :
	path(path)
{
	if (path.empty()) {
		this->path = Config::GetInstance()->GetProfilePath();
	}

	Reload();
}

/**
 * Reload all profiles.
 * @throw ProfileExn The full list of profiles could not be loaded.
 */
void ProfileGallery::Reload()
{
	profiles.clear();

	if (!fs::exists(path)) {
		HR_LOG(info) << "Profile directory does not exist: " <<
			(const char*)Str::PU(path);
		return;
	}

	if (!fs::is_directory(path)) {
		throw ProfileExn("Profile path is not a directory: " +
			(const std::string&)Str::PU(path));
	}

	uuid::string_generator uuidGen;
	OS::dirIter_t dend;
	try {
		HR_LOG(info) << "Loading profiles: " << (const char*)Str::PU(path);
		for (OS::dirIter_t iter{ path }; iter != dend; ++iter) {
			std::string uidStr = iter->path().filename().string();

			uuid::uuid uid;
			try {
				uid = uuidGen(uidStr);
			}
			catch (...) {
				HR_LOG(warning) << "Profile directory is not a UUID: " <<
					(const char*)Str::PU(path);
				continue;
			}

			HR_LOG(debug) << "Found profile: " << uidStr;
			try {
				profiles.emplace_back(std::make_shared<LocalProfile>(uid));
			}
			catch (const ProfileExn &ex) {
				HR_LOG(error) << "Failed to load profile: " << uidStr << ": " <<
					ex.what();
			}
		}
	}
	catch (const std::exception &ex) {
		throw ProfileExn(std::string("Unable to load all profiles: ") +
			ex.what());
	}
}

/**
 * Find a profile by UID.
 * @param uid The UID as a string.
 * @return The profile or @c nullptr if the profile does not exist.
 */
std::shared_ptr<Profile> ProfileGallery::FindUid(const std::string &uid)
{
	try {
		return FindUid(uuid::string_generator()(uid));
	}
	catch (...) {
		HR_LOG(warning) << "Invalid UID: " << uid;
		return {};
	}
}

// We don't expect there will be a lot of profiles, so a linear search
// will do.

/**
 * Find a profile by UID.
 * @param uid The UID.
 * @return The profile or @c nullptr if the profile does not exist.
 */
std::shared_ptr<Profile> ProfileGallery::FindUid(const boost::uuids::uuid &uid)
{
	auto iter = std::find_if(profiles.begin(), profiles.end(),
		[&uid](const std::shared_ptr<Profile> &profile) {
			return profile->GetUid() == uid;
		});
	return (iter != profiles.end()) ? *iter : std::shared_ptr<Profile>{};
}

/**
 * Find a profile by name.
 * @param name The name.
 * @return The profile or @c nullptr if the profile does not exist.
 */
std::shared_ptr<Profile> ProfileGallery::FindName(const std::string &name)
{
	auto iter = std::find_if(profiles.begin(), profiles.end(),
		[&name](const std::shared_ptr<Profile> &profile) {
			return profile->GetName() == name;
		});
	return (iter != profiles.end()) ? *iter : std::shared_ptr<Profile>{};
}

}  // namespace Player
}  // namespace HoverRace
