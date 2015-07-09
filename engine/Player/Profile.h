
// Profile.h
//
// Copyright (c) 2014, 2015 Michael Imamura.
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

#pragma once

#include <boost/uuid/uuid.hpp>

#include "../Display/Color.h"
#include "../Display/Res.h"

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
	namespace Display {
		class Texture;
	}
}

namespace HoverRace {
namespace Player {

/**
 * Base class for player profiles.
 * @author Michael Imamura
 */
class MR_DllDeclare Profile
{
public:
	Profile();
	Profile(const boost::uuids::uuid &uid, const std::string &name,
		Display::Color primaryColor, Display::Color secondaryColor) :
		uid(uid), name(name),
		primaryColor(primaryColor), secondaryColor(secondaryColor) { }
	virtual ~Profile() { }

public:
	/**
	 * Retrieve the globally unique identifier for this profile.
	 * @return The ID (may be @c nil if uninitialized).
	 */
	const boost::uuids::uuid &GetUid() const { return uid; }

	/**
	 * Retrieve the name of the player.
	 * @return The name (never empty).
	 */
	const std::string &GetName() const { return name; }

	Display::Color GetPrimaryColor() const { return primaryColor; }

	Display::Color GetSecondaryColor() const { return secondaryColor; }

	virtual std::shared_ptr<Display::Res<Display::Texture>>
		GetAvatar() const;

public:
	virtual void Save() = 0;

private:
	boost::uuids::uuid uid;
	std::string name;
	Display::Color primaryColor;
	Display::Color secondaryColor;
};

}  // namespace Player
}  // namespace HoverRace

#undef MR_DllDeclare
