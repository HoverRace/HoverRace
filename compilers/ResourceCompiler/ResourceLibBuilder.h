// ResourceLibBuilder.h
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
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
//

#pragma once

#include "../../engine/ObjFacTools/ResourceLib.h"

namespace HoverRace {
	namespace Parcel {
		class ObjStream;
	}
}

namespace HoverRace {
namespace ResourceCompiler {

class ResourceLibBuilder : public ObjFacTools::ResourceLib
{
	// Each module can have its own MR_BitmapLib
	public:
		ResourceLibBuilder();
		~ResourceLibBuilder();

		void AddBitmap(ObjFacTools::ResBitmap *pBitmap);
		void AddActor(ObjFacTools::ResActor *pActor);
		void AddSprite(ObjFacTools::ResSprite *pSprite);
		void AddSound(ObjFacTools::ResShortSound *pSound);
		void AddSound(ObjFacTools::ResContinuousSound *pSound);

		bool Export(const HoverRace::Util::OS::path_t &filename);

};

}  // namespace ResourceCompiler
}  // namespace HoverRace
