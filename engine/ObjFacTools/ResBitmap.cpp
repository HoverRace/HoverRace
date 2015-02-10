// ResBitmap.cpp
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

#include "../Parcel/ObjStream.h"

#include "ResBitmap.h"

namespace HoverRace {
namespace ObjFacTools {

namespace {

int MAX_BITMAP_WIDTH = 4096;
int MAX_BITMAP_HEIGHT = 4096;

}

ResBitmap::ResBitmap(int pResourceId)
{
	mResourceId = pResourceId;
	mWidth = 1;
	mHeight = 1;
	mXRes = 1;
	mYRes = 1;
	mSubBitmapCount = 0;
	mPlainColor = 255;
	mSubBitmapList = NULL;
}

ResBitmap::~ResBitmap()
{
	delete[] mSubBitmapList;
}

int ResBitmap::GetResourceId() const
{
	return mResourceId;
} 

/**
 * The Serialize() function either writes this object into a CArchive or reads
 * the object from the CArchive, depending on the output of
 * CArchive::IsStoring().
 *
 * @param pArchive The archive to write to / read from
 */
void ResBitmap::Serialize(Parcel::ObjStream &pArchive)
{
	int lCounter;

	if(pArchive.IsWriting()) {
		pArchive << mWidth;
		pArchive << mHeight;
		pArchive << mXRes;
		pArchive << mYRes;
		pArchive << mSubBitmapCount;
		pArchive << mPlainColor;

	}
	else {
		delete[]mSubBitmapList;
		mSubBitmapList = NULL;

		pArchive >> mWidth;
		pArchive >> mHeight;
		pArchive >> mXRes;
		pArchive >> mYRes;
		pArchive >> mSubBitmapCount;
		pArchive >> mPlainColor;

		if(mSubBitmapCount > 0) {
			mSubBitmapList = new SubBitmap[mSubBitmapCount];
		}

	}

	for(lCounter = 0; lCounter < mSubBitmapCount; lCounter++) {
		mSubBitmapList[lCounter].Serialize(pArchive);
	}

}

void ResBitmap::SetWidthHeight(int pWidth, int pHeight)
{
	mWidth = pWidth;
	mHeight = pHeight;
}

// MR_Bitmap overloaded functions
int ResBitmap::GetWidth() const
{
	return mWidth;
}

int ResBitmap::GetHeight() const
{
	return mHeight;
}

int ResBitmap::GetMaxXRes() const
{
	return mXRes;
}

int ResBitmap::GetMaxYRes() const
{
	return mYRes;
}

MR_UInt8 ResBitmap::GetPlainColor() const
{
	return mPlainColor;
}

int ResBitmap::GetNbSubBitmap() const
{
	return mSubBitmapCount;
}

int ResBitmap::GetXRes(int pSubBitmap) const
{
	return mSubBitmapList[pSubBitmap].mXRes;
}

int ResBitmap::GetYRes(int pSubBitmap) const
{
	return mSubBitmapList[pSubBitmap].mYRes;
}

int ResBitmap::GetXResShiftFactor(int pSubBitmap) const
{
	return mSubBitmapList[pSubBitmap].mXResShiftFactor;
}

int ResBitmap::GetYResShiftFactor(int pSubBitmap) const
{
	return mSubBitmapList[pSubBitmap].mYResShiftFactor;
}

MR_UInt8 *ResBitmap::GetBuffer(int pSubBitmap) const
{
	return mSubBitmapList[pSubBitmap].mBuffer;
}

MR_UInt8 *ResBitmap::GetColumnBuffer(int pSubBitmap, int pColumn) const
{
	return mSubBitmapList[pSubBitmap].mColumnPtr[pColumn];
}

MR_UInt8 **ResBitmap::GetColumnBufferTable(int pSubBitmap) const
{
	return mSubBitmapList[pSubBitmap].mColumnPtr;
} 

ResBitmap::SubBitmap::SubBitmap()
{
	mBuffer = NULL;
	mColumnPtr = NULL;
}

ResBitmap::SubBitmap::~SubBitmap()
{
	delete[] mBuffer;
	delete[] mColumnPtr;
}

void ResBitmap::SubBitmap::Serialize(Parcel::ObjStream &pArchive)
{
	if(pArchive.IsWriting()) {

		pArchive << mXRes;
		pArchive << mYRes;
		pArchive << mXResShiftFactor;
		pArchive << mYResShiftFactor;
		pArchive << mHaveTransparent;

		if (mXRes < 0 || mXRes > MAX_BITMAP_WIDTH ||
			mYRes < 0 || mYRes > MAX_BITMAP_HEIGHT)
		{
			throw Parcel::ObjStreamExn(
				pArchive.GetName(),
				boost::str(boost::format(
					"Writing invalid ResBitmap size: %dx%d") %
						mXRes % mYRes));
		}

		pArchive.Write(mBuffer, static_cast<size_t>(mXRes * mYRes));
	}
	else {
		delete[] mBuffer;
		delete[] mColumnPtr;

		pArchive >> mXRes;
		pArchive >> mYRes;
		pArchive >> mXResShiftFactor;
		pArchive >> mYResShiftFactor;
		pArchive >> mHaveTransparent;

		if (mXRes < 0 || mXRes > MAX_BITMAP_WIDTH ||
			mYRes < 0 || mYRes > MAX_BITMAP_HEIGHT)
		{
			throw Parcel::ObjStreamExn(
				pArchive.GetName(),
				boost::str(boost::format(
					"Writing invalid ResBitmap size: %dx%d") %
						mXRes % mYRes));
		}
		auto sz = static_cast<size_t>(mXRes * mYRes);

		mBuffer = new MR_UInt8[sz];
		mColumnPtr = new MR_UInt8 *[mXRes];

		MR_UInt8 *lPtr = mBuffer;

		for(int lCounter = 0; lCounter < mXRes; lCounter++) {
			mColumnPtr[lCounter] = lPtr;
			lPtr += mYRes;
		}
		pArchive.Read(mBuffer, sz);
	}
}

}  // namespace ObjFacTools
}  // namespace HoverRace
