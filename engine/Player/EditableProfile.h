
// EditableProfile.h
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

#pragma once

#include "../Display/Color.h"

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
namespace Player {

/**
 * Interface for profile editing.
 * @author Michael Imamura
 */
class MR_DllDeclare EditableProfile
{
public:
	virtual void SetAvatarName(const std::string &avatarName) = 0;
	virtual void SetColor(size_t i, Display::Color color) = 0;
	virtual void SetPrimaryColor(Display::Color color) = 0;
	virtual void SetSecondaryColor(Display::Color color) = 0;

public:
	virtual void Save() = 0;
};

/**
 * Interface for profiles than can be renamed.
 * @author Michael Imamura
 */
class MR_DllDeclare RenamableProfile
{
public:
	/**
	 * Attempt to rename the profile.
	 * This implies saving the profile.
	 * @param name The new name (may not be blank).
	 */
	virtual void Rename(const std::string &name) = 0;
};

}  // namespace Player
}  // namespace HoverRace

#undef MR_DllDeclare
