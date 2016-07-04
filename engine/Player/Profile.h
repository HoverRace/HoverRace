
// Profile.h
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

#pragma once

#include <boost/uuid/uuid.hpp>

#include "../Display/Color.h"
#include "../Display/Res.h"
#include "EditableProfile.h"

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
	namespace Player {
		class AvatarGallery;
	}
}

namespace HoverRace {
namespace Player {

/**
 * Base class for player profiles.
 * @author Michael Imamura
 */
class MR_DllDeclare Profile :
	protected EditableProfile,
	protected RenamableProfile
{
public:
	Profile(std::shared_ptr<AvatarGallery> avatarGallery = {});
	Profile(std::shared_ptr<AvatarGallery> avatarGallery,
		const boost::uuids::uuid &uid) :
		Profile(avatarGallery, uid, "Player", {},
		Display::COLOR_WHITE, Display::COLOR_BLACK)
		{ }
	Profile(std::shared_ptr<AvatarGallery> avatarGallery,
		const boost::uuids::uuid &uid, const std::string &name,
		const std::string &avatarName,
		Display::Color primaryColor, Display::Color secondaryColor) :
		avatarGallery(std::move(avatarGallery)),
		uid(uid), name(name), avatarName(avatarName),
		primaryColor(primaryColor), secondaryColor(secondaryColor) { }
	virtual ~Profile() { }

protected:
	AvatarGallery *GetAvatarGallery() const { return avatarGallery.get(); }

public:
	/**
	 * Retrieve the globally unique identifier for this profile.
	 * @return The ID (may be @c nil if uninitialized).
	 */
	const boost::uuids::uuid &GetUid() const { return uid; }

	const std::string GetUidStr() const;

	/**
	 * Retrieve the name of the player.
	 * @return The name (never empty).
	 */
	const std::string &GetName() const { return name; }
protected:
	virtual void SetName(const std::string &name);

public:
	const std::string &GetAvatarName() const { return avatarName; }
protected:
	virtual void SetAvatarName(const std::string &avatarName);

public:
	Display::Color GetColor(size_t i) const;
	void SetColor(size_t i, Display::Color color);

public:
	Display::Color GetPrimaryColor() const { return primaryColor; }
protected:
	virtual void SetPrimaryColor(Display::Color color);

public:
	Display::Color GetSecondaryColor() const { return secondaryColor; }
protected:
	virtual void SetSecondaryColor(Display::Color color);

protected:
	virtual void Rename(const std::string &name) { SetName(name); }

public:
	virtual std::shared_ptr<Display::Res<Display::Texture>>
		GetAvatar() const;

public:
	/**
	 * Retrieve an editable view of this profile.
	 * @return An editable profile backed by this profile, or
	 *         @c nullptr if this profile cannot be edited.
	 */
	virtual EditableProfile *Edit() { return nullptr; }

	/**
	 * Retrieve an renamable view of this profile.
	 * @return An editable profile backed by this profile, or
	 *         @c nullptr if this profile cannot change the name.
	 */
	virtual RenamableProfile *EditName() { return nullptr; }

protected:
	virtual void Save() = 0;

public:
	static const size_t MAX_NAME_LENGTH = 30;
	static const size_t MAX_AVATAR_NAME_LENGTH = 16;
private:
	std::shared_ptr<AvatarGallery> avatarGallery;
	boost::uuids::uuid uid;
	std::string name;
	std::string avatarName;
	Display::Color primaryColor;
	Display::Color secondaryColor;
};

}  // namespace Player
}  // namespace HoverRace

#undef MR_DllDeclare
