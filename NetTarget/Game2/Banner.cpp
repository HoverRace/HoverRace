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

#include "stdafx.h"

#include "Banner.h"
#include "../Util/StrRes.h"
#include "resource.h"

// Local functions

BOOL decoder(short linewidth);


class ByteStream {
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

class BitmapBuilder {
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
	} else {
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
    } else {
	mSize = pStreamSize;
    }
    mIndex = 0;
}

unsigned char ByteStream::GetNext()
{
    if(mIndex < mSize) {
	return mData[mIndex++];
    } else {
	return 0;
    }
}

short ByteStream::GetNextb()
{
    if(mIndex < mSize) {
	return mData[mIndex++];
    } else {
	return -1;
    }
}


WORD ByteStream::GetWord()
{
    if(mIndex + 1 < mSize) {
	WORD lReturnValue = *(WORD *) (mData + mIndex);
	mIndex += 2;

	return lReturnValue;
    } else {
	return 0;
    }
}

    unsigned char ByteStream::Peek() const const const {
	if(mIndex < mSize)
{
return mData[mIndex];
	} else {
	    return 0;
	}
    }

    BOOL ByteStream::Empty() const const const {
	return (mIndex >= mSize);
    } void ByteStream::Inc(int pCount)
{
    if(mIndex < mSize) {
	mIndex += pCount;
    }
}


MR_GifDecoder::MR_GifDecoder()
{
    mNbImages = 0;
    mGlobalPalette = NULL;

    for(int lCounter = 0; lCounter < MR_MAX_IMAGES; lCounter++) {
	mBitmap[lCounter] = NULL;
    }
}

MR_GifDecoder::~MR_GifDecoder()
{
    Clean();
}

void MR_GifDecoder::Clean()
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


BOOL MR_GifDecoder::Decode(const unsigned char *pGifStream, int pStreamLen)
{
    return FALSE;
}

HPALETTE MR_GifDecoder::GetGlobalPalette() const {
	return mGlobalPalette;
}

int MR_GifDecoder::GetImageCount() const {
	return mNbImages;
}

HBITMAP MR_GifDecoder::GetImage(int pImage) const {
	return mBitmap[pImage];
}

int MR_GifDecoder::GetDelay(int pImage) const {
	return mDelay[pImage];
}

// Link stuff
	
BOOL LoadURL(HWND pWindow, const char *pURL)
{
    BOOL lReturnValue = FALSE;
    char lTempPath[300];

    lTempPath[0] = 0;
    lTempPath[280] = 0;

    GetTempPath(280, lTempPath);

    strcat(lTempPath, "HRBanner.url");

    FILE *lFile = fopen(lTempPath, "w");

    if(lFile != NULL) {
	fprintf(lFile, "[InternetShortcut]\nURL=%s\n", pURL);

	fclose(lFile);

	// Now execute that file
	unsigned int lReturnCode = (int) ShellExecute(pWindow,
						      NULL,
						      lTempPath,
						      NULL,
						      NULL,
						      SW_SHOWNORMAL);

	if(lReturnCode <= 32) {
	    CString lMessage;

	    lMessage.Format(IDS_WWW_REQ, pURL);
	    MessageBox(pWindow, lMessage, MR_LoadString(IDS_ERROR), MB_ICONERROR | MB_APPLMODAL | MB_OK);
	} else {
	    lReturnValue = TRUE;
	}
    }

    return lReturnValue;
}


BOOL LoadURLShortcut(HWND pWindow, const char *pShortcut)
{
    BOOL lReturnValue = FALSE;

    unsigned int lReturnCode = (int) ShellExecute(pWindow,
						  NULL,
						  pShortcut,
						  NULL,
						  NULL,
						  SW_SHOWNORMAL);

    if(lReturnCode > 32) {
	lReturnValue = TRUE;
    } else {
	char lURL[300];

	strcpy(lURL, "http://www.HoverRace.com");	// Default well there is nothing that can be done


	FILE *lFile = fopen(pShortcut, "r");

	if(lFile != NULL) {
	    char lLine[300];

	    while(fgets(lLine, sizeof(lLine), lFile)) {
		sscanf(lLine, " URL = %300s ", lURL);
	    }

	    fclose(lFile);
	}


	CString lMessage;

	lMessage.Format(IDS_WWW_REQ, lURL);
	MessageBox(pWindow, lMessage, MR_LoadString(IDS_ERROR), MB_ICONERROR | MB_APPLMODAL | MB_OK);
    }

    return lReturnValue;
}
