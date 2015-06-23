
// ObjFac1.h
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

namespace HoverRace {
	namespace ObjFacTools {
		class ResourceLib;
	}
	namespace Util {
		class ObjectFromFactory;
	}
}

namespace HoverRace {
namespace ObjFac1 {

class ObjFac1
{
public:
	ObjFac1();
	~ObjFac1();

	ObjFacTools::ResourceLib *GetResourceLib() const { return resourceLib.get(); }

	Util::ObjectFromFactory *GetObject(int pClassId);

private:
	std::unique_ptr<ObjFacTools::ResourceLib> resourceLib;
};

} // namespace ObjFac1
} // namespace HoverRace
