// SpriteHandle.cpp
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

#include "StdAfx.h"

#include "SpriteHandle.h"

namespace HoverRace {
namespace ObjFac1 {

const MR_ResSprite *mSprite;

SpriteHandle::SpriteHandle(const Util::ObjectFromFactoryId & pId, const MR_ResSprite * pSprite) :
	Util::ObjectFromFactory(pId)
{
	mSprite = pSprite;
}

SpriteHandle::~SpriteHandle()
{
}

const MR_Sprite *SpriteHandle::GetSprite() const
{
	return mSprite;
}

}  // namespace ObjFac1
}  // namespace HoverRace
