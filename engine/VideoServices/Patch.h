// Patch.h
//

#ifndef MR_PATCH_H
#define MR_PATCH_H

#include "../Util/WorldCoordinates.h"

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

class MR_DllDeclare MR_Patch
{
	protected:
	public:
		virtual int GetURes() const = 0;
		virtual int GetVRes() const = 0;

		/*
		const MR_3DCoordinate & GetNodePos(int pU, int pV) const;
												  // More effective function
		const MR_3DCoordinate & GetNodePos(int pIndex) const;
												  // Most effective function
		*/
		virtual const MR_3DCoordinate *GetNodeList() const = 0;

};

#undef MR_DllDeclare
#endif
