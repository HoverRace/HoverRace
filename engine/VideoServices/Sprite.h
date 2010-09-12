// Sprite.h
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

#include "Viewport2D.h"
#include "../Util/DllObjectFactory.h"

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
namespace VideoServices {

class MR_DllDeclare Sprite
{
	public:
		enum eAlignment
		{
			eLeft = 0,
			eTop = 0,
			eRight = 1,
			eBottom = 1,
			eCenter = 2
		};

	protected:
		int mNbItem;
		int mItemHeight;
		int mTotalHeight;
		int mWidth;
		MR_UInt8 *mData;

	public:
		Sprite();
		~Sprite();

		void Blt(int pX, int pY, Viewport2D *pDest, eAlignment pHAlign = eLeft, eAlignment pVAlign = eTop, int pItem = 0, int pScaling = 1) const;

		void StrBlt(int pX, int pY, const char *pStr, Viewport2D *pDest, eAlignment pHAlign = eLeft, eAlignment pVAlign = eTop, int pScaling = 1) const;

		int GetNbItem() const;
		int GetItemHeight() const;
		int GetItemWidth() const;

		void Serialize(Parcel::ObjStream &pArchive);

};

// Helper class and functions
MR_DllDeclare const char *Ascii2Simple(const char *pSrc);
MR_DllDeclare char Ascii2Simple(char pSrc);

}  // namespace VideoServices
}  // namespace HoverRace

#undef MR_DllDeclare
