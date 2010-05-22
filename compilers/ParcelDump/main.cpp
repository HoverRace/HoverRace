
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
#	include "../../engine/Parcel/MfcRecordFile.h"
#endif
#include "../../engine/Util/InspectNode.h"

using namespace HoverRace::Parcel;
using namespace HoverRace::Util;

#if defined(_WIN32) && defined(_DEBUG)
#	include <mfcleakfix.h>
	static int foo = use_ignore_mfc_leaks();
#endif

typedef boost::shared_ptr<RecordFile> RecordFilePtr;

static RecordFilePtr OpenRecordFile(const char *filename)
{
	//TODO: Allow loader selection.
	RecordFilePtr file(MfcRecordFile::New());

	if (!file->OpenForRead(filename)) {
		std::cerr << "Invalid or corrupt parcel file: " << filename << std::endl;
		file.reset();
	}

	return file;
}

int main(int argc, char **argv)
{
	if (argc < 2) return EXIT_FAILURE;

	//TODO: Command line options.
	const char *filename = argv[1];

	RecordFilePtr file = OpenRecordFile(filename);
	if (file == NULL) return EXIT_FAILURE;

	InspectNode root;
	file->Inspect(root);
#	ifdef _DEBUG
		std::stringstream oss;
		root.RenderToStream(oss);
		std::string outStr(oss.str());
		std::cout << outStr;
		OutputDebugStringA(outStr.c_str());
		OutputDebugStringA("\n");
#	else
		root.RenderToStream(std::cout);
#	endif
	std::cout << std::endl;

	return EXIT_SUCCESS;
}
