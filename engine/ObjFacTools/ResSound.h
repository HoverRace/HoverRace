// ResSound.h
//
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

#include "../VideoServices/SoundServer.h"

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
	}
}

namespace HoverRace {
namespace ObjFacTools {

class ResShortSound
{
	protected:
		int mResourceId;
		VideoServices::ShortSound *mSound;

		int mNbCopy;
		int mDataLen;
		char *mData;

	public:
												  // Only availlable for resourceLib and construction
		MR_DllDeclare ResShortSound(int pResourceId);
		MR_DllDeclare ~ResShortSound();

		MR_DllDeclare int GetResourceId() const;
		MR_DllDeclare void Serialize(Parcel::ObjStream &pArchive);

		MR_DllDeclare VideoServices::ShortSound *GetSound() const;

};

class ResContinuousSound
{
	protected:
		int mResourceId;
		HoverRace::VideoServices::ContinuousSound *mSound;

		int mNbCopy;
		int mDataLen;
		char *mData;

	public:
												  // Only availlable for resourceLib and construction
		MR_DllDeclare ResContinuousSound(int pResourceId);
		MR_DllDeclare ~ResContinuousSound();

		MR_DllDeclare int GetResourceId() const;
		MR_DllDeclare void Serialize(Parcel::ObjStream &pArchive);

		MR_DllDeclare VideoServices::ContinuousSound *GetSound() const;

};

}  // namespace ObjFacTools
}  // namespace HoverRace

#undef MR_DllDeclare
