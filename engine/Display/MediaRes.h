
// MediaRes.h
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

#pragma once

#include <boost/filesystem/fstream.hpp>

#include "../Util/Config.h"
#include "../Util/OS.h"
#include "../Util/Str.h"

#include "Res.h"

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

/**
 * Represents a file in the game's media directory.
 * @author Michael Imamura
 */
template<class T>
class MediaRes : public Res<T>
{
	typedef Res<T> SUPER;
	public:
		/**
		 * Constructor.
		 * @param path File path, relative to the media directory.
		 */
		MediaRes(const Util::OS::path_t &path) : SUPER(), path(path),
			id(std::string("media:") + (const char*)Util::Str::PU(path)) { }
		virtual ~MediaRes() { }

	public:
		std::string GetId() const override
		{
			return id;
		}

		std::unique_ptr<std::istream> Open() const override
		{
			namespace fs = boost::filesystem;
			auto cfg = Util::Config::GetInstance();
			return std::unique_ptr<std::istream>(new fs::ifstream(
				cfg->GetMediaPath() / path,
				std::ios_base::in | std::ios_base::binary));
		}

	private:
		Util::OS::path_t path;
		std::string id;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
