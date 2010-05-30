
// SoundServer.h
// DirectSound / OpenAL encapsulation module
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

#include <string>

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

class ShortSound;
class ContinuousSound;

namespace SoundServer
{

	MR_DllDeclare bool Init(
#		ifndef WITH_OPENAL
			HWND pWindow
#		endif
		);
	MR_DllDeclare void Close();

	MR_DllDeclare std::string GetInitError();

	MR_DllDeclare ShortSound *CreateShortSound(const char *pData, int pNbCopy);
	MR_DllDeclare void DeleteShortSound(ShortSound * pSound);

	MR_DllDeclare void Play(ShortSound * pSound, int pDB = 0, double pSpeed = 1.0, int pPan = 0);

	MR_DllDeclare int GetNbCopy(ShortSound * pSound);

	// Continous play
	MR_DllDeclare ContinuousSound *CreateContinuousSound(const char *pData, int pNbCopy);
	MR_DllDeclare void DeleteContinuousSound(ContinuousSound * pSound);

	MR_DllDeclare void Play(ContinuousSound * pSound, int pCopy, int pDB = 0, double pSpeed = 1.0, int pPan = 0);

	/* Obsoletes
	   MR_DllDeclare void                Pause(   ContinuousSound* pSound, int pCopy );
	   MR_DllDeclare void                Restart( ContinuousSound* pSound, int pCopy );

	   MR_DllDeclare void                SetParams( ContinuousSound* pSound, int pCopy, int pDB, double pSpeed = 1.0, int pPan = 0 );
	 */
	MR_DllDeclare int GetNbCopy(ContinuousSound * pSound);

	MR_DllDeclare void ApplyContinuousPlay();

};

}  // namespace VideoServices
}  // namespace HoverRace

#undef MR_DllDeclare
