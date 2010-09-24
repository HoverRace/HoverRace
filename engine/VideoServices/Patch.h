// Patch.h
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

namespace HoverRace {
namespace VideoServices {

class MR_DllDeclare Patch
{
	public:
		virtual int GetURes() const = 0;
		virtual int GetVRes() const = 0;

		/*
		const MR_3DCoordinate & GetNodePos(int pU, int pV) const
		{
			return GetNodePos(pU + pV * GetURes());
		}
												  // More effective function
		const MR_3DCoordinate & GetNodePos(int pIndex) const
		{
			return GetNodeList()[pIndex];
		}
												  // Most effective function
		*/
		virtual const MR_3DCoordinate *GetNodeList() const = 0;

};

}  // namespace VideoServices
}  // namespace HoverRace

#undef MR_DllDeclare
