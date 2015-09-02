
// ResourceLib.h
//
// Copyright (c) 2010, 2014, 2015 Michael Imamura.
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

#include "../Util/OS.h"
#include "ResActor.h"
#include "ResSprite.h"
#include "ResSound.h"

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
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

/**
 * Legacy resource manager for ObjFac1.dat resources.
 * @author Michael Imamura
 */
class MR_DllDeclare ResourceLib
{
protected:
	ResourceLib();
public:
	ResourceLib(const Util::OS::path_t &filename);
	~ResourceLib();

public:
	ResBitmap *GetBitmap(int id);
	const ResActor *GetActor(int id);
	const ResSprite *GetSprite(int id);
	const ResShortSound *GetShortSound(int id);
	const ResContinuousSound *GetContinuousSound(int id);

protected:
	std::unique_ptr<Parcel::RecordFile> recordFile;

	std::map<int, std::unique_ptr<ResBitmap>> bitmaps;
	std::map<int, std::unique_ptr<ResActor>> actors;
	std::map<int, std::unique_ptr<ResSprite>> sprites;
	std::map<int, std::unique_ptr<ResShortSound>> shortSounds;
	std::map<int, std::unique_ptr<ResContinuousSound>> continuousSounds;

protected:
	static const MR_UInt32 FILE_MAGIC = 12345;
};

}  // namespace HoverRace
}  // namespace ObjFacTools

#undef MR_DllDeclare
