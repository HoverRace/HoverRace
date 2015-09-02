
// ResBundle.h
//
// Copyright (c) 2015 Michael Imamura.
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

#include "Bundle.h"

namespace HoverRace {
	namespace ObjFacTools {
		class ResourceLib;
	}
}

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
namespace Parcel {

/**
 * Abstraction for loading resources.
 * @author Michael Imamura
 */
class MR_DllDeclare ResBundle : public Bundle
{
	using SUPER = Bundle;

public:
	ResBundle(const Util::OS::path_t &dir,
		std::shared_ptr<ResBundle> subBundle = std::shared_ptr<ResBundle>());
	virtual ~ResBundle();

public:
	ObjFacTools::ResourceLib &GetResourceLib() const { return *resourceLib; }

private:
	std::unique_ptr<ObjFacTools::ResourceLib> resourceLib;
};

}  // namespace Parcel
}  // namespace HoverRace

#undef MR_DllDeclare
