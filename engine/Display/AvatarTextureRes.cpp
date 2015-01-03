
// AvatarTextureRes.cpp
//
// Copyright (c) 2014 Michael Imamura.
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

#include <boost/filesystem/fstream.hpp>

#include "../Util/Config.h"
#include "../Util/OS.h"
#include "../Util/Str.h"

#include "AvatarTextureRes.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

AvatarTextureRes::AvatarTextureRes(const std::string &avatarId) :
	SUPER(),
	avatarId(avatarId), id("avatar:" + avatarId)
{
}

std::unique_ptr<std::istream> AvatarTextureRes::Open() const
{
	namespace fs = boost::filesystem;
	
	auto cfg = Util::Config::GetInstance();

	//TODO: Sanity check avatar ID and return placeholder if necessary.
	//TODO: Check if avatar cached first; schedule download if necessary.
	
	auto imgPath = cfg->GetMediaPath();
	imgPath /= Str::UP("avatars");
	imgPath /= Str::UP(avatarId + ".png");

	return std::unique_ptr<std::istream>(new fs::ifstream(
		imgPath, std::ios_base::in | std::ios_base::binary));
}

}  // namespace Display
}  // namespace HoverRace
