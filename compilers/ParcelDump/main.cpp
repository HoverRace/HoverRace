
// main.cpp
// Debug dumper for parcel files.
//
// Copyright (c) 2010 Michael Imamura.
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

#include "StdAfx.h"

#ifdef _WIN32
#	include <fcntl.h>
#	include <io.h>
#else
#	include <locale.h>
#	include <langinfo.h>
#endif

#include <boost/algorithm/string/predicate.hpp>

#include "../../engine/Model/Track.h"
#include "../../engine/Model/TrackEntry.h"
#include "../../engine/Parcel/RecordFile.h"
#include "../../engine/Parcel/TrackBundle.h"
#include "../../engine/Util/InspectMapNode.h"
#include "../../engine/Util/Str.h"

using namespace HoverRace::Model;
using namespace HoverRace::Parcel;
using namespace HoverRace::Util;
namespace fs = boost::filesystem;

#if 0
#if defined(_WIN32) && defined(_DEBUG)
#	include <mfcleakfix.h>
	static int foo = use_ignore_mfc_leaks();
#endif
#endif

static void InspectAndPrint(const std::string &label, const Inspectable *insp)
{
	InspectMapNode root;
	insp->Inspect(root);

	std::string outStr;
	outStr.reserve(4096);
	root.RenderToString(outStr);

	std::cout << "--- # " << label << std::endl;
	std::cout << outStr << std::endl;
#	if defined(_WIN32) && defined(_DEBUG)
		std::wstring ws((const wchar_t*)Str::UW(outStr.c_str()));
		OutputDebugStringW(L"--- # ");
		OutputDebugStringW(Str::UW(label.c_str()));
		OutputDebugStringW(L"\n");
		OutputDebugStringW(ws.c_str());
		OutputDebugStringW(L"\n");
#	endif
}

static void DumpTrack(const OS::path_t &path, const std::string &name)
{
	TrackBundle trackBundle(path);
	TrackPtr track = trackBundle.OpenTrack(name);
	if (track.get() == NULL) {
		std::cerr << "Corrupted track file." << std::endl;
		return;
	}

	InspectAndPrint("Header", track->GetRecordFile().get());
	InspectAndPrint("Track", track.get());
}

int main(int argc, char **argv)
{
#	ifdef _WIN32
		/*
		_setmode(_fileno(stdout), _O_U8TEXT);
		*/
#	endif
	OS::SetLocale();
#	ifndef _WIN32
		if (strcmp(nl_langinfo(CODESET), "UTF-8") != 0) {
			std::cerr << "Warning: Locale is not UTF-8." << std::endl;
		}
#	endif

	if (argc < 2) return EXIT_FAILURE;

#	ifdef _WIN32
		int wargc;
		wchar_t **wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
		OS::path_t filename(wargv[1]);
#	else
		OS::path_t filename(argv[1]);
#	endif

	if (!fs::exists(filename)) {
		std::cerr << "File does not exist." << std::endl;
		return EXIT_FAILURE;
	}

	OS::path_t dir = filename.parent_path();
	std::string name = Str::PU(filename.filename().c_str());

	if (boost::algorithm::ends_with(name, ".trk")) {
		DumpTrack(dir, name);
	}
	else {
		std::cerr << "-- Unsupported record file type." << std::endl;
	}

	return EXIT_SUCCESS;
}
