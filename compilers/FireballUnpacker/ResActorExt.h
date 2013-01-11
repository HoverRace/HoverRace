// ResActorExt.h
// Provides an extension to the ResActor class to help turn it back into a text file

#ifndef RES_ACTOR_EXT_H
#define RES_ACTOR_EXT_H

#include "StdAfx.h"
#include "../../engine/ObjFacTools/ResActor.h"

class ResActorExt : public MR_ResActor
{
	public:
		ResActorExt();

		int GetComponentCount(int sequence, int frame);
		int GetVertexCount(int sequence, int frame, int patch);
		int GetPatchURes(int sequence, int frame, int patch);
		int GetPatchVRes(int sequence, int frame, int patch);
		int GetPatchBitmapId(int sequence, int frame, int patch);
		MR_3DCoordinate *GetVertexList(int sequence, int frame, int patch);
};

#endif