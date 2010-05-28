
// ResourceLib.h
// Loadable resource manager.
//
// Copyright (c) 2010 Michael Imamura.
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

#pragma once

#include <map>

#include "ResActor.h"
#include "ResSprite.h"
#include "ResSound.h"

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
	namespace Parcel {
		class ObjStream;
		class RecordFile;
	}
}

namespace HoverRace {
namespace ObjFacTools {

class MR_DllDeclare ResourceLib
{
	protected:
		ResourceLib() { }
	public:
		ResourceLib(const char *filename);
		~ResourceLib();

	public:
		MR_ResBitmap * GetBitmap(int id);
		const MR_ResActor *GetActor(int id);
		const MR_ResSprite *GetSprite(int id);
		const MR_ResShortSound *GetShortSound(int id);
		const MR_ResContinuousSound *GetContinuousSound(int id);

	protected:
		Parcel::RecordFile *recordFile;

		typedef std::map<int, MR_ResBitmap*> bitmaps_t;
		bitmaps_t bitmaps;
		typedef std::map<int, MR_ResActor*> actors_t;
		actors_t actors;
		typedef std::map<int, MR_ResSprite*> sprites_t;
		sprites_t sprites;
		typedef std::map<int, MR_ResShortSound*> shortSounds_t;
		shortSounds_t shortSounds;
		typedef std::map<int, MR_ResContinuousSound*> continuousSounds_t;
		continuousSounds_t continuousSounds;
};

#define MR_RESOURCE_FILE_MAGIC    12345

}  // namespace HoverRace
}  // namespace ObjFacTools

#undef MR_DllDeclare
