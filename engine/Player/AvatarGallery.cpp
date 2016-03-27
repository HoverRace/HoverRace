
// AvatarGallery.cpp
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

#include "../Display/MediaRes.h"
#include "../Display/Texture.h"
#include "../Util/Config.h"
#include "../Util/Log.h"
#include "../Util/Str.h"

#include "AvatarGallery.h"

namespace fs = boost::filesystem;
using namespace HoverRace::Util;

namespace HoverRace {
namespace Player {

/**
 * Constructor.
 * @param path The base avatar directory (may be empty to use default).
 */
AvatarGallery::AvatarGallery(const OS::path_t &path) :
	path(path)
{
	if (path.empty()) {
		this->path = Config::GetInstance()->GetMediaPath() / "avatars";
	}

	Reload();
}

/**
 * Reload all avatars.
 */
void AvatarGallery::Reload()
{
	avatars.clear();

	if (!fs::exists(path)) {
		HR_LOG(info) << "Avatar directory does not exist: " <<
			(const char*)Str::PU(path);
		return;
	}

	if (!fs::is_directory(path)) {
		HR_LOG(warning) << "Avatar directory is not a directory: " <<
			(const char*)Str::PU(path);
		return;
	}

	OS::dirIter_t dend;
	for (OS::dirIter_t iter{ path }; iter != dend; ++iter) {
		auto filename = iter->path().filename();
		auto avatarName = filename.stem().string();

		HR_LOG(debug) << "Found avatar: " << avatarName << ": " << filename;
		avatars.emplace(
			avatarName,
			std::make_shared<Display::MediaRes<Display::Texture>>(
				Str::UP("avatars") / filename));
	}
}

/**
 * Retrieve the name of the default avatar.
 * @return The avatar name (never empty).
 */
const std::string &AvatarGallery::GetDefaultAvatarName()
{
	static const std::string DEFAULT_NAME("_default");
	return DEFAULT_NAME;
}

/**
 * Find the avatar for a given name.
 * @param s The avatar name.
 * @return The avatar, or @c nullptr if not found.
 */
std::shared_ptr<Display::Res<Display::Texture>> AvatarGallery::FindName(
	const std::string &s) const
{
	auto iter = avatars.find(s);
	if (iter == avatars.end()) {
		HR_LOG(debug) << "Unknown avatar: " << s;
		return {};
	}
	else {
		return iter->second;
	}
}

}  // namespace Player
}  // namespace HoverRace
