// Sprite.cpp
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

#include "../StdAfx.h"

#include "../Parcel/ObjStream.h"

#include "Sprite.h"

namespace HoverRace {
namespace VideoServices {

Sprite::Sprite()
{
	mNbItem = 0;
	mData = NULL;
}

Sprite::~Sprite()
{
	if(mData != NULL) {
		delete[]mData;
	}
}

int Sprite::GetNbItem() const
{
	return mNbItem;
}

int Sprite::GetItemHeight() const
{
	return mItemHeight;
}

int Sprite::GetItemWidth() const
{
	return mWidth;
}

void Sprite::Serialize(Parcel::ObjStream &pArchive)
{
	using namespace Parcel;

	if(pArchive.IsWriting()) {
		pArchive << mNbItem;
		pArchive << mItemHeight;
		pArchive << mTotalHeight;
		pArchive << mWidth;

		pArchive.Write(mData, static_cast<size_t>(mWidth * mTotalHeight));
	}
	else {
		delete[]mData;
		mData = NULL;

		pArchive >> mNbItem;
		pArchive >> mItemHeight;
		pArchive >> mTotalHeight;
		pArchive >> mWidth;

		if (mNbItem <= 0) throw ObjStreamExn("mNbItem must be > 0");
		if (mItemHeight <= 0) throw ObjStreamExn("mItemHeight must be > 0");
		if (mTotalHeight <= 0) throw ObjStreamExn("mTotalHeight must be > 0");
		if (mWidth <= 0) throw ObjStreamExn("mWidth must be > 0");

		size_t totalSz = static_cast<size_t>(mWidth * mTotalHeight);
		mData = new MR_UInt8[totalSz];
		pArchive.Read(mData, totalSz);

	}
}

void Sprite::Blt(int pX, int pY, Viewport2D *pDest, eAlignment pHAlign, eAlignment pVAlign, int pItem, int pScaling) const
{
	ASSERT((pItem < mNbItem) && (pItem >= 0));

	if(pItem < mNbItem) {

		int lScaledWidth = mWidth / pScaling;
		int lScaledHeight = mItemHeight / pScaling;
		int lScaledLineStep = mWidth * pScaling;

		switch (pHAlign) {
			case eLeft:
				// Do nothing.
				break;

			case eRight:
				pX -= lScaledWidth;
				break;

			case eCenter:
				pX -= lScaledWidth / 2;
				break;
		}

		switch (pVAlign) {
			case eTop:
				// Do nothing.
				break;

			case eBottom:
				pY -= lScaledHeight;
				break;

			case eCenter:
				pY -= lScaledHeight / 2;
				break;
		}

		if(pX < 0) {
			pX = 0;
		}
		if(pY < 0) {
			pY = 0;
		}

		int lDestStep = pDest->GetLineLen();
		MR_UInt8 *lDest = pDest->GetBuffer() + pX + lDestStep * pY;
		MR_UInt8 *lSrc = mData + mWidth * mItemHeight * pItem;

		int lBltLen = pDest->GetXRes() - pX;
		int lBltHeight = pDest->GetYRes() - pY;

		if(lBltLen > lScaledWidth) {
			lBltLen = lScaledWidth;
		}

		if(lBltHeight > lScaledHeight) {
			lBltHeight = lScaledHeight;
		}

		for(int lY = 0; lY < lBltHeight; lY++) {
			for(int lSrcX = 0, lDestX = 0; lDestX < lBltLen; lDestX++, lSrcX += pScaling) {
				if(lSrc[lSrcX] != 0) {
					lDest[lDestX] = lSrc[lSrcX];
				}
			}
			lDest += lDestStep;
			lSrc += lScaledLineStep;
		}
	}
}

void Sprite::StrBlt(int pX, int pY, const char *pStr, Viewport2D *pDest, eAlignment pHAlign, eAlignment pVAlign, int pScaling) const
{
	const size_t MAX_LEN = 1024;

	if((pStr != NULL) && (pStr[0] != 0)) {
		size_t slen = strlen(pStr);
		int lStrLen = static_cast<int>(std::min(slen, MAX_LEN));
		int lStep;

		if(pScaling == 0) {
			// Auto scale
			lStep = mWidth * 3 / 4;

			int lStrWidth = lStrLen * lStep;

			if(lStrWidth > pDest->GetXRes()) {
				pScaling = 1 + lStrWidth / pDest->GetXRes();
				lStep /= pScaling;
			}
			else {
				pScaling = 1;
			}
		}
		else {
			lStep = mWidth * 3 / (4 * pScaling);
		}

		switch (pHAlign) {
			case eLeft:
				// Do nothing.
				break;

			case eRight:
				pX -= lStrLen * lStep;
				break;

			case eCenter:
				pX -= lStrLen * lStep / 2;
				break;
		}

		switch (pVAlign) {
			case eTop:
				// Do nothing.
				break;

			case eBottom:
				pY -= mItemHeight / pScaling;
				break;

			case eCenter:
				pY -= mItemHeight / (2 * pScaling);
				break;
		}

		while(*pStr != 0) {
			Blt(pX, pY, pDest, eLeft, eTop, *pStr, pScaling);

			pStr++;
			pX += lStep;
		}
	}
}

// Helper class and functions
const char *Ascii2Simple(const char *pSrc)
{
	// Warning: non reentrant function (not for multi thread)

	// Conversion string
	// " !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
	static char lBuffer[256];

	if(pSrc == NULL) {
		lBuffer[0] = 0;
	}
	else {
		unsigned lCounter = 0;
		while((lCounter < (sizeof(lBuffer) - 1)) && (*pSrc != 0)) {
			if((*pSrc >= 32) && (*pSrc < 127)) {
				lBuffer[lCounter++] = (char) (*pSrc - 32 + 1);
			}
			else {
				lBuffer[lCounter++] = '_' - 32 + 1;
			}
			pSrc++;
		}
		lBuffer[lCounter] = 0;
	}

	return lBuffer;

}

char Ascii2Simple(char pSrc)
{
	char lReturnValue = 0;

	if ((pSrc >= 32) && (pSrc < 127)) {
		lReturnValue = static_cast<char>(static_cast<int>(pSrc) - 32 + 1);
	}
	return lReturnValue;
}

}  // namespace VideoServices
}  // namespace HoverRace
