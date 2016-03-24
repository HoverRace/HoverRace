
// AvatarGallery.h
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
 * The collection of built-in avatars.
 * @author Michael Imamura
 */
class MR_DllDeclare AvatarGallery
{
public:
	AvatarGallery(const Util::OS::path_t &path = {});

public:
	void Reload();

public:
	bool IsEmpty() const { return avatars.empty(); }

public:
	std::shared_ptr<Display::Res<Display::Texture>> FindName(
		const std::string &s) const;

private:
	using avatars_t = std::unordered_map<
		std::string,
		std::shared_ptr<Display::Res<Display::Texture>>>;
public:
	// STL-like iteration, so we can use range-for.
	using iterator = avatars_t::iterator;
	using const_iterator = avatars_t::const_iterator;
	using value_type = avatars_t::value_type;
	const_iterator begin() const { return avatars.begin(); }
	iterator begin() { return avatars.begin(); }
	const_iterator end() const { return avatars.end(); }
	iterator end() { return avatars.end(); }

private:
	Util::OS::path_t path;
	avatars_t avatars;
};

}  // namespace Player
}  // namespace HoverRace

#undef MR_DllDeclare
