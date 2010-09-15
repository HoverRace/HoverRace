// SpriteHandle.h
//
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

// The class defined in this file can be used as a base class for
// Sprites
//
//

#pragma once

#include "ResourceLib.h"

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
namespace ObjFac1 {

class SpriteHandle : public Util::ObjectFromFactory
{
	protected:
		const ObjFacTools::ResSprite *mSprite;

	public:
		MR_DllDeclare SpriteHandle(const Util::ObjectFromFactoryId & pId, const ObjFacTools::ResSprite * pSprite);
		MR_DllDeclare ~SpriteHandle();

		MR_DllDeclare const VideoServices::Sprite *GetSprite() const;

};

}  // namespace ObjFac1
}  // namespace HoverRace

#undef MR_DllDeclare
