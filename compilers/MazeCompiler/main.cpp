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

#include "StdAfx.h"

#include <boost/filesystem/fstream.hpp>

#include "../../engine/Model/TrackFileCommon.h"
#include "../../engine/Parcel/ClassicRecordFile.h"
#include "../../engine/Parcel/ObjStream.h"
#include "../../engine/Util/Config.h"
#include "../../engine/Util/OS.h"
#include "../../engine/VideoServices/SoundServer.h"

#include "../../engine/MazeCompiler/LevelBuilder.h"
#include "../../engine/MazeCompiler/MapSprite.h"
#include "../../engine/MazeCompiler/TrackSpecParser.h"

#ifdef WITH_SDL
#	ifdef _WIN32
#		ifdef _DEBUG
#			pragma comment(lib, "SDLmainD")
#		else
#			pragma comment(lib, "SDLmain")
#		endif
#	endif
#endif

using namespace HoverRace;
using namespace HoverRace::MazeCompiler;
using namespace HoverRace::Util;

static void PrintUsage()
{
	// this should be redone, it's horrible
	puts(_("Usage: MazeCompiler <outputfile> <inputfile>"));
}

static bool CreateHeader(std::istream &in, Parcel::ObjStream &pDestination);
static bool AddBackgroundImage(std::istream &in, Parcel::ObjStream &pDestination);

static std::string FormatStr(const char *pSrc);
static MR_UInt8 *PCXRead(FILE * pFile, int &pXRes, int &pYRes);
static MR_UInt8 *LoadBitmap(FILE * pFile);
static MR_UInt8 *LoadPalette(FILE * pFile);

int main(int pArgCount, char *pArgStrings[])
{
	bool lPrintUsage = false;
	bool lError = false;

	//TODO: Process command-line options.

	Config *cfg = Config::Init(0, 0, 0, 0, true, OS::path_t());
	cfg->runtime.silent = true;

#	ifdef ENABLE_NLS
		// Gettext initialization.
		OS::SetLocale();
		bind_textdomain_codeset(PACKAGE, "UTF-8");
		bindtextdomain(PACKAGE, LOCALEDIR);
		textdomain(PACKAGE);
#	endif

	puts("HoverRace Track Compiler      (c)1996-1997 GrokkSoft Inc.");

	VideoServices::SoundServer::Init();
	Util::DllObjectFactory::Init();

#	ifdef _WIN32
		int wargc;
		wchar_t **wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
#	endif

	OS::path_t outputFilename;
	OS::path_t inputFilename;

	// Analyse the input parameters
	if(pArgCount != 3) {
		lPrintUsage = true;
		puts(_("Wrong argument count"));
	}
	else {
#		ifdef _WIN32
			outputFilename = wargv[1];
			inputFilename = wargv[2];
#		else
			outputFilename = pArgStrings[1];
			inputFilename = pArgStrings[2];
#		endif
	}

	if(!lError && !lPrintUsage) {
		Parcel::ClassicRecordFile lOutputFile;

		// Try to create the output file
		if(!lOutputFile.CreateForWrite(outputFilename, 4, "\x8\rHoverRace track file\n\x1a")) {
			lError = true;
			puts(_("Unable to create the output file"));
		}
		// Compile each level
		printf(_("Compiling track (%s)"), pArgStrings[2]);
		printf("\n");

		// Open the input file
		std::ifstream in = boost::filesystem::ifstream(inputFilename);
		if (in.fail()) {
			lError = true;
			puts(_("Unable to open the input file"));
		}

		// Compile the level
		LevelBuilder *lNewLevel = new LevelBuilder;

		if(!lError) {
			if(!lOutputFile.BeginANewRecord()) {
				lError = true;
				puts(_("Unable to add a header to the output file"));
			}
			else {
				Parcel::ObjStreamPtr archivePtr(lOutputFile.StreamOut());
				Parcel::ObjStream &lArchive = *archivePtr;

				lError = !CreateHeader(in, lArchive);
			}
			in.seekg(0, std::ios::beg);
		}

		if(!lError) {
			// Parse the input file
			// and amze the preprocessing
			if(!lNewLevel->InitFromStream(in)) {
				lError = TRUE;
				puts(_("Track creation error"));
			}
			in.seekg(0, std::ios::beg);
		}

		// Add the level to the file
		if(!lError) {
			if(!lOutputFile.BeginANewRecord()) {
				lError = TRUE;
				puts(_("Unable to add the track to the output file"));
			}
			else {
				Parcel::ObjStreamPtr archivePtr(lOutputFile.StreamOut());
				Parcel::ObjStream &lArchive = *archivePtr;

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
				Parcel::ObjStreamPtr archivePtr(lOutputFile.StreamOut());
				Parcel::ObjStream &lArchive = *archivePtr;

				lError = !AddBackgroundImage(in, lArchive);
			}
			in.seekg(0, std::ios::beg);
		}

		if(!lError) {
			// Add the Map
			if(!lOutputFile.BeginANewRecord()) {
				lError = TRUE;
				puts(_("Unable to add a map record"));
			}
			else {
				Parcel::ObjStreamPtr archivePtr(lOutputFile.StreamOut());
				Parcel::ObjStream &lArchive = *archivePtr;

				int lX0;
				int lX1;
				int lY0;
				int lY1;

				MapSprite lMapSprite;

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
	}

	/*TODO
	if(!lError && !lPrintUsage) {
		// Apply checksum.
		Parcel::ClassicRecordFile lOutputFile;
		lOutputFile.ReOpen(outputFilename);
	}
	*/

	if(lPrintUsage) {
		if(lError) {
			puts(_("Command line error"));
		}
		PrintUsage();
	}

	Util::DllObjectFactory::Clean(FALSE);
	VideoServices::SoundServer::Close();

	Config::Shutdown();

	return lError ? 255 : 0;
}

bool CreateHeader(std::istream &in, Parcel::ObjStream &pArchive)
{
	bool lReturnValue = true;

	std::string lDescription;
	int lSortingOrder = 50;
	int lRegistration = MR_REGISTRED_TRACK;
	TrackSpecParser lParser(in);

#ifdef _DEBUG
	lSortingOrder = 30;
	lRegistration = MR_FREE_TRACK;
#else
	lSortingOrder = 40;
#endif

	if(lParser.GetNextClass("HEADER") == NULL) {
		lReturnValue = false;
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
		pArchive << (int) 0;  // Registration minor ID (obsolete)
		pArchive << (int) 0;  // Registration major ID (obsolete)
		pArchive << lSortingOrder;
		pArchive << lRegistration;
		if(lRegistration == MR_FREE_TRACK) {
												  // Hehe dont try to modify the file or it will crash :-)
			pArchive << (int) MR_MAGIC_TRACK_NUMBER;
		}
	}

	return lReturnValue;

}

std::string FormatStr(const char *pSrc)
{
	std::string lReturnValue;
	bool lEsc = false;

	while(*pSrc != 0) {
		if(lEsc) {
			lEsc = false;

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
				lEsc = true;
			}
			else {
				lReturnValue += *pSrc;
			}
		}
		pSrc++;
	}
	return lReturnValue;
}

bool AddBackgroundImage(std::istream &in, Parcel::ObjStream &pDestination)
{
	bool lReturnValue = true;

	std::string lBackFileName;

	TrackSpecParser lParser(in);

	if(lParser.GetNextClass("HEADER") == NULL) {
		lReturnValue = false;
		puts(_("Unable to find [HEADER] section"));
	}
	else {
		if(!lParser.GetNextAttrib("Background")) {
			lReturnValue = false;
			puts(_("Unable to find background image"));
		}
		else {
			lBackFileName = lParser.GetParams();
		}
	}

	if(lReturnValue) {
		FILE *lBackFile = fopen(lBackFileName.c_str(), "rb");

		if(lBackFile == NULL) {
			lReturnValue = false;
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
