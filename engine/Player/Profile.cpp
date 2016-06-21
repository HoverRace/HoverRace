
// Profile.cpp
//
// Copyright (c) 2014-2016 Michael Imamura.
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

#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <utf8/utf8.h>

#include "../Display/MediaRes.h"
#include "AvatarGallery.h"

#include "Profile.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Player {

Profile::Profile(std::shared_ptr<AvatarGallery> avatarGallery) :
	Profile(avatarGallery, boost::uuids::nil_uuid())
{
}

/**
 * Retrieve the UID as a string.
 * @return The UID (never blank).
 */
const std::string Profile::GetUidStr() const
{
	return boost::uuids::to_string(uid);
}

void Profile::SetName(const std::string &name)
{
	if (utf8::distance(name.cbegin(), name.cend()) > MAX_NAME_LENGTH) {
		auto divIter = name.cbegin();
		utf8::advance(divIter, MAX_NAME_LENGTH, name.cend());
		this->name.assign(name.cbegin(), divIter);
	}
	else {
		this->name = name;
	}
}

void Profile::SetAvatarName(const std::string &avatarName)
{
	//TODO: Trim and validate avatar name.
	this->avatarName = avatarName;
}

void Profile::SetPrimaryColor(Display::Color color)
{
	primaryColor = color;
	primaryColor.bits.a = 0xff;  // Must be fully-opaque.
}

void Profile::SetSecondaryColor(Display::Color color)
{
	secondaryColor = color;
	secondaryColor.bits.a = 0xff;  // Must be fully-opaque.
}

/**
 * Retrieve the user's avatar, if available.
 * @return The avatar resource (may be @c nullptr).
 */
std::shared_ptr<Display::Res<Display::Texture>> Profile::GetAvatar() const
{
	if (!avatarGallery || avatarName.empty()) {
		return {};
	}
	else {
		return avatarGallery->FindName(avatarName);
	}
}

}  // namespace Player
}  // namespace HoverRace
