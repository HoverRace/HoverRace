
// ProfileGallery.h
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

#include "../Util/OS.h"
#include "Profile.h"

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
 * The collection of player profiles on this system.
 * @author Michael Imamura
 */
class MR_DllDeclare ProfileGallery
{
	using SUPER = Profile;

public:
	ProfileGallery(const Util::OS::path_t &path = {});

public:
	void Reload();

public:
	std::shared_ptr<Profile> FindUid(const std::string &uid);
	std::shared_ptr<Profile> FindUid(const boost::uuids::uuid &uid);
	std::shared_ptr<Profile> FindName(const std::string &name);

private:
	using profiles_t = std::vector<std::shared_ptr<Profile>>;
public:
	// STL-like iteration, so we can use range-for.
	using iterator = profiles_t::iterator;
	using const_iterator = profiles_t::const_iterator;
	using value_type = profiles_t::value_type;
	const_iterator begin() const { return profiles.begin(); }
	iterator begin() { return profiles.begin(); }
	const_iterator end() const { return profiles.end(); }
	iterator end() { return profiles.end(); }

private:
	Util::OS::path_t path;
	profiles_t profiles;
};

}  // namespace Player
}  // namespace HoverRace

#undef MR_DllDeclare
