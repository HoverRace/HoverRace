// ResShortSoundExt.h
// Extension for the ResShortSound class so that more information can be obtained for extraction

#ifndef RES_SHORT_SOUND_EXT_H
#define RES_SHORT_SOUND_EXT_H

#include "StdAfx.h"
#include "../../engine/ObjFacTools/ResSound.h"

class ResShortSoundExt : public MR_ResShortSound
{
	public:
		//ResShortSoundExt();

		unsigned long GetDataLength();
		char *GetData();
		int GetNbCopy();
};

#endif