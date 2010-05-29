// main.cpp                              MazeCompiler main file
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
#include "../../engine/Parcel/MfcRecordFile.h"
#include "LevelBuilder.h"
#include "TrackMap.h"
#include "Parser.h"
#include "TrackCommonStuff.h"
#include "../../engine/Parcel/ObjStream.h"
#include "resource.h"

#include <stdio.h>

#define new DEBUG_NEW

using namespace HoverRace::Parcel;

static void PrintUsage()
{
	// this should be redone, it's horrible
	puts(_("Usage: MazeCompiler <outputfile> <inputfile>"));
}

static BOOL CreateHeader(FILE * pInputFile, ObjStream &pDestination);
static BOOL AddBackgroundImage(FILE * pInputFile, ObjStream &pDestination);

static CString FormatStr(const char *pSrc);
static MR_UInt8 *PCXRead(FILE * pFile, int &pXRes, int &pYRes);
static MR_UInt8 *LoadBitmap(FILE * pFile);
static MR_UInt8 *LoadPalette(FILE * pFile);

static char gOwner[81];
static int gMajorID = 0;
static int gMinorID = 0;
static unsigned char gKey[50];

int main(int pArgCount, const char **pArgStrings)
{
	BOOL lPrintUsage = FALSE;
	BOOL lError = FALSE;

	
	printf(_("HoverRace Track Compiler"));
	printf("      ");
	printf(_("(c)1996-1997 GrokkSoft Inc."));
	printf("\n");

	Sleep(1000);
	MR_DllObjectFactory::Init();

	// Analyse the input parameters
	if(pArgCount != 3) {
		lPrintUsage = TRUE;
		puts(_("Wrong argument count"));
		Sleep(4000);
	}

	if(!lError && !lPrintUsage) {
		if((gMajorID != 0) && (gMajorID != 100)) {
			lError = TRUE;

			const char *lStr = strrchr(pArgStrings[1], '[');

			if(lStr != NULL) {
				int lMajorID = -2;
				int lMinorID = -2;

				sscanf(lStr + 1, "%d-%d", &lMajorID, &lMinorID);

				if((lMajorID == gMajorID) && (lMinorID == gMinorID)) {
					lError = FALSE;
				}
			}

			if(lError) {
				puts(_("Track name must contain your registration number"));
			}
		}
	}

	if(!lError && !lPrintUsage) {
		MfcRecordFile lOutputFile;

		// Verify that there is at least one ofhtr parameter
		CString lCopyrightNotice = "\x8\r" + CString(_("HoverRace track file")) + ", " + "(c)GrokkSoft 1997\n\x1a";

		if((gMajorID != 0) && (gMajorID != 100)) {
			lCopyrightNotice.Format("\x8\r" + CString(_("HoverRace track file designed by %s(%d-%d)")) + "\n\x1a", gOwner, gMajorID, gMinorID);
		}
		// Try to create the output file
		if(!lOutputFile.CreateForWrite(pArgStrings[1], 4, lCopyrightNotice)) {
			lError = TRUE;
			puts(_("Unable to create the output file"));
		}
		// Compile each level
		printf(_("Compiling track (%s)"), pArgStrings[2]);
		printf("\n");

		// Open the input file
		FILE *lFile = fopen(pArgStrings[2], "r");

		if(lFile == NULL) {
			lError = TRUE;

			puts(_("Unable to open the input file"));
		}
		// Compile the level
		MR_LevelBuilder *lNewLevel = new MR_LevelBuilder;

		if(!lError) {
			if(!lOutputFile.BeginANewRecord()) {
				lError = TRUE;
				puts(_("Unable to add a header to the output file"));
			}
			else {
				ObjStreamPtr archivePtr(lOutputFile.StreamOut());
				ObjStream &lArchive = *archivePtr;

				lError = !CreateHeader(lFile, lArchive);
			}
			rewind(lFile);
		}

		if(!lError) {
			// Parse the input file
			// and amze the preprocessing
			if(!lNewLevel->InitFromFile(lFile)) {
				lError = TRUE;
				puts(_("Track creation error"));
			}
			rewind(lFile);
		}
		// Add the level to the file
		if(!lError) {
			if(!lOutputFile.BeginANewRecord()) {
				lError = TRUE;
				puts(_("Unable to add the track to the output file"));
			}
			else {
				ObjStreamPtr archivePtr(lOutputFile.StreamOut());
				ObjStream &lArchive = *archivePtr;

				lNewLevel->Serialize(lArchive);
			}
		}

		if(!lError) {
			// Add the background file
			if(!lOutputFile.BeginANewRecord()) {
				lError = TRUE;
				puts(_("Unable to add a background image record"));
			}
			else {
				ObjStreamPtr archivePtr(lOutputFile.StreamOut());
				ObjStream &lArchive = *archivePtr;

				lError = !AddBackgroundImage(lFile, lArchive);
			}
			rewind(lFile);
		}

		if(!lError) {
			// Add the Map
			if(!lOutputFile.BeginANewRecord()) {
				lError = TRUE;
				puts(_("Unable to add a map record"));
			}
			else {
				ObjStreamPtr archivePtr(lOutputFile.StreamOut());
				ObjStream &lArchive = *archivePtr;

				int lX0;
				int lX1;
				int lY0;
				int lY1;

				MR_MapSprite lMapSprite;

				lMapSprite.CreateMap(lNewLevel, lX0, lY0, lX1, lY1);

				lArchive << lX0;
				lArchive << lX1;
				lArchive << lY0;
				lArchive << lY1;

				lMapSprite.Serialize(lArchive);

			}
		}

		delete lNewLevel;

		if(!lError) {
			// Print the statistics on the output file
			printf(_("The output file now contains %d records"), lOutputFile.GetNbRecords());
			printf("\n");
		}

		if(lFile != NULL) {
			fclose(lFile);
		}

	}

	if(!lError) {
		MfcRecordFile lOutputFile;
		lOutputFile.ApplyChecksum(pArgStrings[1]);
	}

	if(lPrintUsage) {
		if(lError) {
			puts(_("Command line error"));
		}
		PrintUsage();
	}

	if(lError) {
		Sleep(7000);
	}

	Sleep(2000);
	// printf( "END\n" );

	MR_DllObjectFactory::Clean(FALSE);

	return lError ? 255 : 0;
}

BOOL CreateHeader(FILE * pInputFile, ObjStream &pArchive)
{
	BOOL lReturnValue = TRUE;

	CString lDescription;
	int lSortingOrder = 50;
	int lRegistration = MR_REGISTRED_TRACK;
	MR_Parser lParser(pInputFile);

	// Look in the registry to find the User name and member number

#ifdef _DEBUG
	lSortingOrder = 30;
	lRegistration = MR_FREE_TRACK;
#else
	if(gMajorID == 0) {
		lSortingOrder = 40;
	}
#endif

	if(lParser.GetNextClass("HEADER") == NULL) {
		lReturnValue = FALSE;
		puts(_("Unable to find [HEADER] section"));
	}
	else {
		if(!lParser.GetNextAttrib("Description")) {
			// don't crash, use empty description
			lDescription = "";
			puts(_("Warning: unable to find description"));
		}
		else {
			lDescription = FormatStr(lParser.GetParams());
		}

	}

	if(lReturnValue) {
		pArchive << (int) MR_MAGIC_TRACK_NUMBER;
		pArchive << (int) 1;					  // version
		pArchive << lDescription;
		pArchive << (int) gMinorID;
		pArchive << (int) gMajorID;
		pArchive << lSortingOrder;
		pArchive << lRegistration;
		if(lRegistration == MR_FREE_TRACK) {
												  // Hehe dont try to modify the file or it will crash :-)
			pArchive << (int) MR_MAGIC_TRACK_NUMBER;
		}
	}

	return lReturnValue;

}

CString FormatStr(const char *pSrc)
{
	CString lReturnValue;
	BOOL lEsc = FALSE;

	while(*pSrc != 0) {
		if(lEsc) {
			lEsc = FALSE;

			switch (*pSrc) {
				case 'n':
					lReturnValue += '\n';
					break;

				case '\\':
					lReturnValue += '\\';
					break;
			}
		}
		else {
			if(*pSrc == '\\') {
				lEsc = TRUE;
			}
			else {
				lReturnValue += *pSrc;
			}
		}
		pSrc++;
	}
	return lReturnValue;
}

BOOL AddBackgroundImage(FILE * pInputFile, ObjStream &pDestination)
{
	BOOL lReturnValue = TRUE;

	CString lBackFileName;

	MR_Parser lParser(pInputFile);

	if(lParser.GetNextClass("HEADER") == NULL) {
		lReturnValue = FALSE;
		puts(_("Unable to find [HEADER] section"));
	}
	else {
		if(!lParser.GetNextAttrib("Background")) {
			lReturnValue = FALSE;
			puts(_("Unable to find background image"));
		}
		else {
			lBackFileName = lParser.GetParams();

		}
	}

	if(lReturnValue) {
		FILE *lBackFile = fopen(lBackFileName, "rb");

		if(lBackFile == NULL) {
			lReturnValue = FALSE;
			puts(_("Unable to find background image"));
		}
		else {
			// Extract the palette and image from the input file
			MR_UInt8 *lBitmap = LoadBitmap(lBackFile);
			MR_UInt8 *lPalette = LoadPalette(lBackFile);

			if((lBitmap != NULL) && (lPalette != NULL)) {
				pDestination << (int) MR_RAWBITMAP;
				pDestination.Write(lPalette, MR_BACK_COLORS * 3);
				pDestination.Write(lBitmap, MR_BACK_X_RES * MR_BACK_Y_RES);
			}
			else {
				puts(_("Unable to find background image"));
			}

			delete[]lBitmap;
			delete[]lPalette;

			fclose(lBackFile);
		}
	}
	return lReturnValue;
}

MR_UInt8 *LoadPalette(FILE * pFile)
{
	MR_UInt8 *lReturnValue = new MR_UInt8[MR_BACK_COLORS * 3];

	fseek(pFile, -769, SEEK_END);

	char lMagicNumber = 0;

	fread(&lMagicNumber, 1, 1, pFile);

	if(lMagicNumber != 12) {
		puts(_("Bad palette format for background file"));
		delete[]lReturnValue;
		lReturnValue = NULL;
	}
	else {
		fread(lReturnValue, 1, MR_BACK_COLORS * 3, pFile);
	}

	return lReturnValue;
}

MR_UInt8 *LoadBitmap(FILE * pFile)
{
	MR_UInt8 *lReturnValue = NULL;

	int pXRes;
	int pYRes;

	lReturnValue = PCXRead(pFile, pXRes, pYRes);

	if(lReturnValue) {
		if((pXRes != MR_BACK_X_RES) || (pYRes != MR_BACK_Y_RES)) {
			puts(_("Incorrect background resolution"));
			delete[]lReturnValue;
			lReturnValue = NULL;
		}
	}

	if(lReturnValue != NULL) {
		// Make the invertion of the image pixels and add the pixel offset
		MR_UInt8 *lOldBuffer = lReturnValue;

		lReturnValue = new MR_UInt8[pXRes * pYRes];

		int lDestIndex = 0;

		for(int lX = 0; lX < pXRes; lX++) {
			MR_UInt8 *lSource = lOldBuffer + lX;

			for(int lY = pYRes - 1; lY >= 0; lY--) {
				if(*lSource >= 128) {
					// To correct a bug of Paint Shop Pro
					*lSource = 0;
				}
				lReturnValue[lDestIndex + lY] = (*lSource) + MR_RESERVED_COLORS_BEGINNING + MR_BASIC_COLORS;
				lSource += pXRes;
			}
			lDestIndex += pYRes;
		}
		delete lOldBuffer;
	}
	return lReturnValue;
}

MR_UInt8 *PCXRead(FILE * pFile, int &pXRes, int &pYRes)
{
	MR_UInt8 *lReturnValue = NULL;

	MR_Int16 lStartX;
	MR_Int16 lStartY;
	MR_Int16 lSizeX;
	MR_Int16 lSizeY;
	MR_Int16 lNbBytesPerLine;

	fseek(pFile, 4, SEEK_SET);
	fread(&lStartX, 2, 1, pFile);
	fread(&lStartY, 2, 1, pFile);
	fread(&lSizeX, 2, 1, pFile);
	fread(&lSizeY, 2, 1, pFile);

	fseek(pFile, 66, SEEK_SET);
	fread(&lNbBytesPerLine, 2, 1, pFile);

	pXRes = lSizeX - lStartX + 1;
	pYRes = lSizeY - lStartY + 1;

	fseek(pFile, 128, SEEK_SET);

	ASSERT(pXRes > 0);
	ASSERT(pYRes > 0);

	lReturnValue = new MR_UInt8[pXRes * pYRes];

	int lOffset = 0;

	for(int lLineNb = 0; lLineNb < pYRes; lLineNb++) {
		for(int lColNb = 0; lColNb < lNbBytesPerLine;) {
			MR_UInt8 lBuffer;
			MR_UInt8 lBuffer2;

			fread(&lBuffer, 1, 1, pFile);

			if((lBuffer & 0xc0) == 0xc0) {
				fread(&lBuffer2, 1, 1, pFile);
				/*
				   lBuffer2++;
				   if( lBuffer2 == 255 )
				   {
				   lBuffer2 = 0;
				   }
				 */

				for(int lCounter = 0; lCounter < (lBuffer & 0x3f); lCounter++) {
					if(lColNb < pXRes) {
						lReturnValue[lOffset++] = lBuffer2;
					}
					lColNb++;
				}
			}
			else {
				if(lColNb < pXRes) {
					/*
					   lBuffer++;
					   if( lBuffer2 == 255 )
					   {
					   lBuffer2 = 0;
					   }
					 */
					lReturnValue[lOffset++] = lBuffer;
				}
				lColNb++;
			}
		}
	}

	return lReturnValue;
}
