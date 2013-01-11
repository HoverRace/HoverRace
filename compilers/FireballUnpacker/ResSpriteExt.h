// ResSpriteExt.h
// an extension class to ResSprite so I can actually get the data

#ifndef RES_SPRITE_EXT_H
#define RES_SPRITE_EXT_H

#include "StdAfx.h"
#include "../../engine/ObjFacTools/ResSprite.h"

class ResSpriteExt : public MR_ResSprite
{
	public:
		MR_UInt8 *GetData();
		int GetTotalHeight();
};

#endif