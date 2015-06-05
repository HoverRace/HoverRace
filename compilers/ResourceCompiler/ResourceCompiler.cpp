// Resource compiler.cpp
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

// This tool compiles bitmaps and meshings associateds to an
// object factory in a binary file used at run time

// CommandLine
//
//    ResourceCompiler <InputFile> <OutputFile> [<DefineFile>]
//
//
//
// Input File format
//
//    The input file format can only contains 2 keywords
//
//     BITMAP or ACTOR or SPRITE
//
//  The format of a valid line is
//
//     <Keyword> <ExternalFile> <NumericId of the resource>
//
//
//
//  DefineFileFormat
//
//     The define file is used to make a string to int relation. This way all
//     numeric constants defined in input files can be expressed in the
//     form of a string like in C
//
//     The valid lines in this file must have the following form
//     #define <Character string> <Integer constant>
//

#ifdef _WIN32
#	include <shellapi.h>
#endif

#include "TextParsing.h"
#include "ResBitmapBuilder.h"
#include "ResActorBuilder.h"
#include "ResSpriteBuilder.h"
#include "ResSoundBuilder.h"
#include "ResourceLibBuilder.h"
#include "../../engine/ColorTools/ColorTools.h"
#include "../../engine/Util/Str.h"

using namespace HoverRace::ResourceCompiler;
using namespace HoverRace::Util;

// Local functions prototypes
static BOOL ParseInputFile(const OS::path_t &pFileName);
static void PrintUsage();

// Local data
static ResourceLibBuilder gsLib;

int main(int pArgc, char **pArgs)
{
	BOOL lReturnValue = TRUE;

	printf("Resources Compiler               (C)Copyright GrokkSoft inc, 1996\n");

	HoverRace::ColorTools::Init();

#	ifdef ENABLE_NLS
		// Gettext initialization.
		OS::SetLocale();
		bind_textdomain_codeset(PACKAGE, "UTF-8");
		bindtextdomain(PACKAGE, LOCALEDIR);
		textdomain(PACKAGE);
#	endif

#	ifdef _WIN32
		HR_UNUSED(pArgs);
		int wargc;
		wchar_t **wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
#	endif

	if(pArgc <= 2) {
		PrintUsage();
	}
	else {
		if(pArgc >= 4) {
#			ifdef _WIN32
				lReturnValue = MR_ReadPredefinedConstants(wargv[3]);
#			else
				lReturnValue = MR_ReadPredefinedConstants(pArgs[3]);
#			endif
		}

		if(lReturnValue) {
#			ifdef _WIN32
				lReturnValue = ParseInputFile(wargv[1]);
#			else
				lReturnValue = ParseInputFile(pArgs[1]);
#			endif
		}

		if(lReturnValue) {
#			ifdef _WIN32
				lReturnValue = gsLib.Export(wargv[2]);
#			else
				lReturnValue = gsLib.Export(pArgs[2]);
#			endif
		}
	}

	return (lReturnValue ? 0 : -1);
}

void PrintUsage()
{
	printf("\nResourceCompiler <InputFile> <OutputFile> [<DefineFile>]\n\n");
}

BOOL ParseInputFile(const OS::path_t &pFileName)
{
	using namespace HoverRace::ObjFacTools;

	static const char *lKeywordList[] = { "BITMAP", "ACTOR", "SPRITE", "SHORT_SOUND", "CONT_SOUND", NULL };

	BOOL lReturnValue = TRUE;

	FILE *lFile = OS::FOpen(pFileName, "r");

	if(lFile == NULL) {
		lReturnValue = FALSE;
		fprintf(stderr, "%s: %s (%s).\n", _("ERROR"), _("unable to open input file"), (const char*)Str::PU(pFileName));
	}
	else {
		char lLineBuffer[250];

		while(lReturnValue && fgets(lLineBuffer, sizeof(lLineBuffer), lFile)) {
			int lResourceId;
			int lNbItem;
			int lNbCopy;
			float lWidth;
			float lHeight;
			int lAntiAliasScheme;

			std::string lLine = MR_PreProcLine(lLineBuffer);

			switch (MR_ContainsKeyword(lLine.c_str(), lKeywordList)) {
				case 0:  // BITMAP
					if(sscanf(lLine.c_str(), " %*s %249s %d %f %f %d", lLineBuffer, &lResourceId, &lWidth, &lHeight, &lAntiAliasScheme) != 5) {
						fprintf(stderr, "%s: %s.\n", _("ERROR"), _("problem reading bitmap parameters"));
						lReturnValue = FALSE;
					}
					else {
						std::unique_ptr<ResBitmapBuilder> lBitmap(
							new ResBitmapBuilder(lResourceId,
								(int)(lWidth * 1000), (int)(lHeight * 1000)));

						lReturnValue = lBitmap->BuildFromFile(lLineBuffer, lAntiAliasScheme);

						if(lReturnValue) {
							gsLib.AddBitmap(std::move(lBitmap));
						}
					}
					break;

				case 1:  // ACTOR
					if(sscanf(lLine.c_str(), " %*s %249s %d ", lLineBuffer, &lResourceId) != 2) {
						fprintf(stderr, "%s: %s.\n", _("ERROR"), _("problem reading actor parameters"));
					}
					else {
						std::unique_ptr<ResActorBuilder> lActor(
							new ResActorBuilder(lResourceId));

						lReturnValue = lActor->BuildFromFile(lLineBuffer, &gsLib);

						if(lReturnValue) {
							gsLib.AddActor(std::move(lActor));
						}
					}
					break;

				case 2:  // SPRITE
					if(sscanf(lLine.c_str(), " %*s %249s %d %d", lLineBuffer, &lResourceId, &lNbItem) != 3) {
						fprintf(stderr, "%s: %s.\n", _("ERROR"), _("problem reading sprite parameters"));
						lReturnValue = FALSE;
					}
					else {
						std::unique_ptr<ResSpriteBuilder> lSprite(
							new ResSpriteBuilder(lResourceId));

						lReturnValue = lSprite->BuildFromFile(lLineBuffer, lNbItem);

						if(lReturnValue) {
							gsLib.AddSprite(std::move(lSprite));
						}
					}
					break;

				case 3:  // SHORT_SOUND
					if(sscanf(lLine.c_str(), " %*s %249s %d %d", lLineBuffer, &lResourceId, &lNbCopy) != 3) {
						fprintf(stderr, "%s: %s.\n", _("ERROR"), _("problem reading sound parameters"));
						lReturnValue = FALSE;
					}
					else {
						std::unique_ptr<ResShortSoundBuilder> lSound(
							new ResShortSoundBuilder(lResourceId));

						lReturnValue = lSound->BuildFromFile(lLineBuffer, lNbCopy) ? TRUE : FALSE;

						if(lReturnValue) {
							gsLib.AddSound(std::move(lSound));
						}
					}
					break;

				case 4:  // CONT_SOUND
					if(sscanf(lLine.c_str(), " %*s %249s %d %d", lLineBuffer, &lResourceId, &lNbCopy) != 3) {
						fprintf(stderr, "%s: %s.\n", _("ERROR"), _("problem reading sound parameters"));
						lReturnValue = FALSE;
					}
					else {
						std::unique_ptr<ResContinuousSoundBuilder> lSound(
							new ResContinuousSoundBuilder(lResourceId));

						lReturnValue = lSound->BuildFromFile(lLineBuffer, lNbCopy) ? TRUE : FALSE;

						if(lReturnValue) {
							gsLib.AddSound(std::move(lSound));
						}
					}
					break;

			}

		}
		fclose(lFile);
	}

	return lReturnValue;
}
