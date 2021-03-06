
// ResBundle.cpp
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

#include "../ObjFacTools/ResourceLib.h"
#include "../Util/Config.h"
#include "../Util/Str.h"

#include "ResBundle.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Parcel {

ResBundle::ResBundle(const Util::OS::path_t &dir,
	std::shared_ptr<ResBundle> subBundle) :
	SUPER(dir, subBundle),
	resourceLib()
{
}

ResBundle::~ResBundle()
{
}

/**
 * Unload all loaded resources.
 * This should be performed on exit (before the subsystems shut down), and
 * whenever it would be beneficial to free memory to load new resources.
 */
void ResBundle::FreeResources()
{
	resourceLib.reset();
}

ObjFacTools::ResourceLib &ResBundle::GetResourceLib() const
{
	if (!resourceLib) {
		resourceLib.reset(
			new ObjFacTools::ResourceLib(
				Config::GetInstance()->GetMediaPath("ObjFac1.dat")));
	}
	return *resourceLib;
}

}  // namespace Parcel
}  // namespace HoverRace
