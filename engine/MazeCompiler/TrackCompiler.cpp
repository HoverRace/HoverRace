// TrackCompiler.cpp
//
// Copyright (c) 2013, 2015 Michael Imamura.
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

#include <boost/filesystem/fstream.hpp>
#include <boost/scoped_array.hpp>

#include "../Model/TrackFileCommon.h"
#include "../Parcel/ClassicRecordFile.h"
#include "../Parcel/ObjStream.h"
#include "../Util/Str.h"
#include "../Exception.h"
#include "LevelBuilder.h"
#include "MapSprite.h"
#include "TrackCompileExn.h"
#include "TrackSpecParser.h"

#include "TrackCompiler.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace MazeCompiler {

namespace {
	// Temporary until this is rewritten.
	void sfread(void *buf, size_t sz, size_t num, FILE *file)
	{
		if (fread(buf, sz, num, file) < num) {
			if (feof(file)) {
				throw Exception("Unexpected end of file");
			}
			else if (ferror(file)) {
				throw Exception("Read error");
			}
			else {
				throw Exception("Unknown read error");
			}
		}
	}
}

TrackCompiler::TrackCompiler(const TrackCompilationLogPtr &log, const Util::OS::path_t &outputFilename) :
	outputFilename(outputFilename), log(log)
{
}

/**
 * Compile the track from a text file.
 * @param inputFilename The full path to the input file.
 * @throws TrackCompileExn
 */
void TrackCompiler::Compile(const Util::OS::path_t &inputFilename) const
{
	boost::filesystem::ifstream in(inputFilename);
	if (in.fail()) {
		throw TrackCompileExn(boost::str(
			boost::format(_("Unable to open the input file: %s")) %
				Str::PU(inputFilename)));
	}

	Compile(in);
}

/**
 * Compile the track from an input stream.
 * @param in The input stream.
 * @throws TrackCompileExn
 */
void TrackCompiler::Compile(std::istream &in) const
{
	Parcel::ClassicRecordFile outFile;

	// Try to create the output file
	if (!outFile.CreateForWrite(outputFilename, 4, "\x8\rHoverRace track file\n\x1a")) {
		throw TrackCompileExn(boost::str(
			boost::format(_("Unable to create the output file: %s")) %
				Str::PU(outputFilename)));
	}
	log->Info(boost::str(
		boost::format(_("Compiling track (%s)")) %
			Str::PU(outputFilename)));

	// Header.
	if (!outFile.BeginANewRecord()) {
		throw TrackCompileExn(_("Unable to add a header to the output file"));
	}
	else {
		Parcel::ObjStreamPtr archivePtr(outFile.StreamOut());
		Parcel::ObjStream &archive = *archivePtr;
		CreateHeader(in, archive);
	}
	in.clear();
	in.seekg(0, std::ios::beg);

	// The track itself.
	LevelBuilder builder(log);
	if (!outFile.BeginANewRecord()) {
		throw TrackCompileExn(_("Unable to add the track to the output file"));
	}
	else {
		if (!builder.InitFromStream(in)) {
			throw TrackCompileExn(_("Track creation error"));
		}

		Parcel::ObjStreamPtr archivePtr(outFile.StreamOut());
		Parcel::ObjStream &archive = *archivePtr;
		builder.Serialize(archive);
	}
	in.clear();
	in.seekg(0, std::ios::beg);

	// The background image.
	if (!outFile.BeginANewRecord()) {
		throw TrackCompileExn(_("Unable to add a background image record"));
	}
	else {
		Parcel::ObjStreamPtr archivePtr(outFile.StreamOut());
		Parcel::ObjStream &archive = *archivePtr;
		AddBackgroundImage(in, archive);
	}
	in.clear();
	in.seekg(0, std::ios::beg);

	// The map sprite.
	if (!outFile.BeginANewRecord()) {
		throw TrackCompileExn(_("Unable to add a map record"));
	}
	else {
		Parcel::ObjStreamPtr archivePtr(outFile.StreamOut());
		Parcel::ObjStream &archive = *archivePtr;

		MapSprite mapSprite;

		int x0, x1, y0, y1;
		mapSprite.CreateMap(&builder, x0, y0, x1, y1);

		archive << x0;
		archive << x1;
		archive << y0;
		archive << y1;

		mapSprite.Serialize(archive);
	}

	log->Info(_("Compiled track successfully!"));
}

void TrackCompiler::CreateHeader(std::istream &in, Parcel::ObjStream &archive) const
{
	std::string desc;
	int sortOrder = 50;
	int lRegistration = MR_REGISTRED_TRACK;
	TrackSpecParser lParser(in);

#ifdef _DEBUG
	sortOrder = 30;
	lRegistration = MR_FREE_TRACK;
#else
	sortOrder = 40;
#endif

	if (lParser.GetNextClass("HEADER") == NULL) {
		throw TrackCompileExn(_("Unable to find [HEADER] section"));
	}

	if(!lParser.GetNextAttrib("Description")) {
		// Don't crash, use empty description.
		desc = "";
		log->Warn(_("Warning: unable to find description"));
	}
	else {
		desc = FormatStr(lParser.GetParams());
	}

	archive << (int) MR_MAGIC_TRACK_NUMBER;
	archive << (int) 1;  // Version.
	archive << desc;
	archive << (int) 0;  // Registration minor ID (obsolete).
	archive << (int) 0;  // Registration major ID (obsolete).
	archive << sortOrder;
	archive << lRegistration;
	if (lRegistration == MR_FREE_TRACK) {
		archive << (int) MR_MAGIC_TRACK_NUMBER;
	}
}

void TrackCompiler::AddBackgroundImage(std::istream &in, Parcel::ObjStream &archive)
{
	TrackSpecParser parser(in);

	if(parser.GetNextClass("HEADER") == NULL) {
		throw TrackCompileExn(_("Unable to find [HEADER] section"));
	}
	if(!parser.GetNextAttrib("Background")) {
		throw TrackCompileExn(_("Unable to find background image filename"));
	}
	std::string bgFilename = parser.GetParams();

	FILE *bgFile = fopen(bgFilename.c_str(), "rb");
	if (bgFile == NULL) {
		throw TrackCompileExn(boost::str(
			boost::format(_("Unable to find background image: %s")) %
				bgFilename));
	}

	// Extract the palette and image from the input file
	try {
		boost::scoped_array<MR_UInt8> bitmap(LoadBitmap(bgFile));
		boost::scoped_array<MR_UInt8> palette(LoadPalette(bgFile));

		archive << (int) MR_RAWBITMAP;
		archive.Write(palette.get(), MR_BACK_COLORS * 3);
		archive.Write(bitmap.get(), MR_BACK_X_RES * MR_BACK_Y_RES);
	}
	catch (...) {
		fclose(bgFile);
		throw;
	}

	fclose(bgFile);
}

std::string TrackCompiler::FormatStr(const char *pSrc)
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

// Background image loading bits.
// This is more-or-less unmodified from the original implementation in
// MazeCompiler, waiting until we have a more robust shared image loader.

MR_UInt8 *TrackCompiler::LoadPalette(FILE * pFile)
{
	MR_UInt8 *lReturnValue = NULL;

	fseek(pFile, -769, SEEK_END);

	char lMagicNumber = 0;

	sfread(&lMagicNumber, 1, 1, pFile);

	if(lMagicNumber != 12) {
		throw TrackCompileExn(_("Bad palette format for background file"));
	}
	else {
		lReturnValue = new MR_UInt8[MR_BACK_COLORS * 3];
		sfread(lReturnValue, 1, MR_BACK_COLORS * 3, pFile);
	}

	return lReturnValue;
}

MR_UInt8 *TrackCompiler::LoadBitmap(FILE * pFile)
{
	MR_UInt8 *lReturnValue = NULL;

	int pXRes;
	int pYRes;

	lReturnValue = PCXRead(pFile, pXRes, pYRes);

	if(lReturnValue) {
		if((pXRes != MR_BACK_X_RES) || (pYRes != MR_BACK_Y_RES)) {
			throw TrackCompileExn(_("Incorrect background resolution"));
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
				lReturnValue[lDestIndex + lY] = static_cast<MR_UInt8>(
					(*lSource) +
					MR_RESERVED_COLORS_BEGINNING + MR_BASIC_COLORS);
				lSource += pXRes;
			}
			lDestIndex += pYRes;
		}
		delete lOldBuffer;
	}
	return lReturnValue;
}

MR_UInt8 *TrackCompiler::PCXRead(FILE * pFile, int &pXRes, int &pYRes)
{
	MR_UInt8 *lReturnValue = NULL;

	MR_Int16 lStartX;
	MR_Int16 lStartY;
	MR_Int16 lSizeX;
	MR_Int16 lSizeY;
	MR_Int16 lNbBytesPerLine;

	fseek(pFile, 4, SEEK_SET);
	sfread(&lStartX, 2, 1, pFile);
	sfread(&lStartY, 2, 1, pFile);
	sfread(&lSizeX, 2, 1, pFile);
	sfread(&lSizeY, 2, 1, pFile);

	fseek(pFile, 66, SEEK_SET);
	sfread(&lNbBytesPerLine, 2, 1, pFile);

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

			sfread(&lBuffer, 1, 1, pFile);

			if((lBuffer & 0xc0) == 0xc0) {
				sfread(&lBuffer2, 1, 1, pFile);
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

}  // namespace MazeCompiler
}  // namespace HoverRace
