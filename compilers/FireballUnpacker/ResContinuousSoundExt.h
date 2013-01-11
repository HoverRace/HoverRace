// ResContinuousSoundExt.h
// extension class to MR_ResContinuousSound

#ifndef RES_CONTINUOUS_SOUND_EXT_H
#define RES_CONTINUOUS_SOUND_EXT_H

#include "StdAfx.h"
#include "../../engine/ObjFacTools/ResSound.h"

class ResContinuousSoundExt : public MR_ResContinuousSound
{
	public:
		//ResContinuousSoundExt();

		unsigned long GetDataLength();
		char *GetData();
		int GetNbCopy();
};

#endif