
// DemoProfile.cpp
//
// Copyright (c) 2014, 2016 Michael Imamura.
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

#include "../Display/MediaRes.h"
#include "AvatarGallery.h"

#include "DemoProfile.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Player {

namespace {

// {a7626e92-26e2-417c-8d40-3b77f24815f0}
const boost::uuids::uuid DEMO_UUID = {{
	0xa7, 0x62, 0x6e, 0x92,
	0x26, 0xe2,
	0x41, 0x7c,
	0x8d, 0x40,
	0x3b, 0x77, 0xf2, 0x48, 0x15, 0xf0
}};

}  // namespace

DemoProfile::DemoProfile(std::shared_ptr<AvatarGallery> avatarGallery) :
	SUPER(avatarGallery, DEMO_UUID, "Player", {},
		0xffd8063b, Display::COLOR_BLACK)
{
}

std::shared_ptr<Display::Res<Display::Texture>> DemoProfile::GetAvatar() const
{
	if (const auto avatarGallery = GetAvatarGallery()) {
		//TODO: Rotate through the available built-in avatars.
		return avatarGallery->FindName("icon1");
	}
	else {
		return {};
	}
}

}  // namespace Player
}  // namespace HoverRace
