// main.cpp                              MazeCompiler main file
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
// Copyright (c) 2012, 2013 Michael Imamura.
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

#include <iostream>

#include "../../engine/Util/Config.h"
#include "../../engine/Util/DllObjectFactory.h"
#include "../../engine/Util/OS.h"
#include "../../engine/Util/Str.h"
#include "../../engine/VideoServices/SoundServer.h"

#include "../../engine/MazeCompiler/TrackCompilationLog.h"
#include "../../engine/MazeCompiler/TrackCompileExn.h"
#include "../../engine/MazeCompiler/TrackCompiler.h"

#ifdef WITH_SDL
#	ifdef _WIN32
#		ifdef _DEBUG
#			pragma comment(lib, "SDLmainD")
#		else
#			pragma comment(lib, "SDLmain")
#		endif
#	endif
#endif

#ifdef _WIN32
	// Ignore the unreferenced pArgStrings parameter.
#	pragma warning(disable: 4100)
#endif

using namespace HoverRace;
using namespace HoverRace::MazeCompiler;
using namespace HoverRace::Util;

namespace {
	struct CompilationLog : public MazeCompiler::TrackCompilationLog {
		virtual void Info(const std::string &msg) {
#			ifdef _WIN32
				std::wcout << static_cast<const wchar_t*>(Str::UW(msg.c_str())) << std::endl;
#			else
				std::cout << msg << std::endl;
#			endif
		}
		virtual void Warn(const std::string &msg) {
#			ifdef _WIN32
				std::wcerr << static_cast<const wchar_t*>(Str::UW(msg.c_str())) << std::endl;
#			else
				std::cerr << msg << std::endl;
#			endif
		}
	};
}

static void PrintUsage()
{
	// this should be redone, it's horrible
	puts(_("Usage: MazeCompiler <outputfile> <inputfile>"));
}

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

	TrackCompilationLogPtr compileLog(new CompilationLog);

	if (!lError && !lPrintUsage) {
		try {
			TrackCompiler(compileLog, outputFilename).Compile(inputFilename);
		}
		catch (TrackCompileExn &ex) {
			lError = true;
			compileLog->Warn(ex.what());
		}
		catch (Exception &ex) {
			lError = true;
			std::ostringstream oss;
			oss <<
				"***\n\n" <<
				_("You found a bug!") << "\n\n" <<
				ex.what() << "\n\n" 
				"***\n\n";
			compileLog->Warn(oss.str());
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
