
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

#include "../Display/MediaRes.h"

#include "Profile.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Player {

Profile::Profile() :
	Profile(boost::uuids::nil_uuid())
{
}

void Profile::SetName(const std::string &name)
{
	this->name = name;
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
	// Use the built-in avatar if specified.
	return avatarName.empty() ?
		std::shared_ptr<Display::Res<Display::Texture>>{} :
		std::make_shared<Display::MediaRes<Display::Texture>>(
			"avatars/" + avatarName);
}

}  // namespace Player
}  // namespace HoverRace
