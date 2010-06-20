// Banner.cpp
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

#include "StdAfx.h"

#include "Banner.h"
#include "GameApp.h"
#include "../../engine/Util/OS.h"
#include "../../engine/Util/Str.h"
#include "resource.h"

#include <olectl.h>

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

/*CRUFT
class ByteStream
{
	private:
		const unsigned char *mData;
		int mSize;
		int mIndex;

	public:

		void Init(const unsigned char *pStream, int pStreamSize);

		unsigned char GetNext();
		short GetNextb();
		WORD GetWord();
		unsigned char Peek() const;

		void Inc(int pCount = 1);

		BOOL Empty() const;
};

class BitmapBuilder
{
	private:
		unsigned char *mBuffer;
		int mLineLen;
		int mNbRow;
		int mTransparentColor;
		BOOL mInterlace;

		int mCurrentLine;

	public:
		void Init(unsigned char *pBuffer, int pLineLen, int pNbRow, int pTransparentColor, BOOL pInterlace);

		short AddLine(unsigned char *pData, int pDataLen);

};

static ByteStream gStream;
static BitmapBuilder gBitmap;

void BitmapBuilder::Init(unsigned char *pBuffer, int pLineLen, int pNbRow, int pTransparentColor, BOOL pInterlace)
{
	mBuffer = pBuffer;
	mLineLen = pLineLen;
	mNbRow = pNbRow;
	mTransparentColor = pTransparentColor;
	mInterlace = pInterlace;
	mCurrentLine = 0;
}

short BitmapBuilder::AddLine(unsigned char *pData, int pDataLen)
{
	if(mBuffer != NULL) {
		if(pDataLen > mLineLen) {
			pDataLen = mLineLen;
		}

		int lLine;

		if(mInterlace) {
			int lCurrentLine = mCurrentLine;
			lLine = lCurrentLine * 8;

			if(lLine >= mNbRow) {
				lCurrentLine -= (mNbRow + 7) / 8;

				lLine = 4 + lCurrentLine * 8;

				if(lLine >= mNbRow) {
					lCurrentLine -= (mNbRow + 3) / 8;

					lLine = 2 + lCurrentLine * 4;

					if(lLine >= mNbRow) {
						lCurrentLine -= (mNbRow + 1) / 4;

						lLine = 1 + lCurrentLine * 2;
					}
				}
			}
		}
		else {
			lLine = mCurrentLine;
		}

		if((lLine < 0) || (lLine >= mNbRow)) {
			// ASSERT( FALSE );
			lLine = 0;
		}

		unsigned char *lDest = mBuffer + mLineLen * lLine;

		for(int lCounter = 0; lCounter < pDataLen; lCounter++) {
			if(pData[lCounter] != mTransparentColor) {
				lDest[lCounter] = pData[lCounter];
			}
		}

		mCurrentLine++;

	}
	return 0;
}

void ByteStream::Init(const unsigned char *pStream, int pStreamSize)
{
	mData = pStream;

	if(mData == NULL) {
		mSize = 0;
	}
	else {
		mSize = pStreamSize;
	}
	mIndex = 0;
}

unsigned char ByteStream::GetNext()
{
	if(mIndex < mSize) {
		return mData[mIndex++];
	}
	else {
		return 0;
	}
}

short ByteStream::GetNextb()
{
	if(mIndex < mSize) {
		return mData[mIndex++];
	}
	else {
		return -1;
	}
}

WORD ByteStream::GetWord()
{
	if(mIndex + 1 < mSize) {
		WORD lReturnValue = *(WORD *) (mData + mIndex);
		mIndex += 2;

		return lReturnValue;
	}
	else {
		return 0;
	}
}

unsigned char ByteStream::Peek() const
{
	if(mIndex < mSize) {
		return mData[mIndex];
	}
	else {
		return 0;
	}
}

BOOL ByteStream::Empty() const
{
	return (mIndex >= mSize);
}

void ByteStream::Inc(int pCount)
{
	if(mIndex < mSize) {
		mIndex += pCount;
	}
}
*/

GifDecoder::GifDecoder()
{
	mNbImages = 0;
	mGlobalPalette = NULL;

	for(int lCounter = 0; lCounter < MR_MAX_IMAGES; lCounter++) {
		mBitmap[lCounter] = NULL;
	}
}

GifDecoder::~GifDecoder()
{
	Clean();
}

void GifDecoder::Clean()
{
	mNbImages = 0;

	if(mGlobalPalette != NULL) {
		UnrealizeObject(mGlobalPalette);
		DeleteObject(mGlobalPalette);
		mGlobalPalette = NULL;
	}

	for(int lCounter = 0; lCounter < MR_MAX_IMAGES; lCounter++) {
		if(mBitmap[lCounter] != NULL) {
			DeleteObject(mBitmap[lCounter]);
			mBitmap[lCounter] = NULL;
			mDelay[lCounter] = 0;
		}
	}
}

/**
 * Load a GIF image.
 * This will replace the currently-loaded image.
 * Animated GIFs are not yet supported -- only the first frame of an
 * animated GIF will be loaded.
 * @param pGifStream The GIF data buffer (may not be NULL).
 * @param pStreamLen The length of the buffer.
 * @return @c true if the load succeeded, @c false otherwise (current
 *         image will still be cleared).
 */
bool GifDecoder::Decode(const unsigned char *pGifStream, int pStreamLen)
{
	// Delete current image.
	Clean();

	bool retv = false;
	HGLOBAL buf = GlobalAlloc(GPTR, pStreamLen);
	memcpy((void*)buf, pGifStream, pStreamLen);

	IStream *stream = NULL;
	IPicture *pic = NULL;

	// We currently don't support animated GIFs, so set big delay for
	// the first frame.
	mDelay[0] = 3600000;

	// Use OleLoadPicture() to convert the GIF stream to an HBITMAP.
	if (SUCCEEDED(CreateStreamOnHGlobal(buf, false, &stream))) {
		if (SUCCEEDED(OleLoadPicture(stream, 0, false, IID_IPicture, (void**)&pic))) {
			HBITMAP hb = NULL;
			pic->get_Handle((OLE_HANDLE*)&hb);
			mBitmap[0] = (HBITMAP)CopyImage(hb, IMAGE_BITMAP, 0, 0, LR_COPYRETURNORG);
			retv = true;
		}
	}

	if (pic != NULL) pic->Release();
	if (stream != NULL) stream->Release();
	GlobalFree(buf);

	return retv;
}

HPALETTE GifDecoder::GetGlobalPalette() const
{
	return mGlobalPalette;
}

/**
 * Retrieve the number of frames in the current image.
 * @return Either zero (no image loaded) or one.
 */
int GifDecoder::GetImageCount() const
{
	return mNbImages;
}

/**
 * Retrieve a frame from the current image.
 * @param pImage The frame index (zero is the first frame).
 * @return The bitmap data, or NULL if no frame at that index.
 */
HBITMAP GifDecoder::GetImage(int pImage) const
{
	return mBitmap[pImage];
}

/**
 * Retrieve the amount of time to display the frame before
 * the next frame should be displayed.
 * @param pImage The frame index (zero is the first frame).
 * @return The delay (in ms) (never negative).
 */
int GifDecoder::GetDelay(int pImage) const
{
	return mDelay[pImage];
}

}  // namespace Client
}  // namespace HoverRace
