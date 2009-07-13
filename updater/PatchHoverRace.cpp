// HoverRace Auto-Updater
// PatchHoverRace.cpp
//
// The actual code that is called to patch HoverRace with bsdiff-generated
// patches.
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

#include "PatchHoverRace.h"

int PatchHoverRace(string targetDir, string patchFile) {
	// we assume that the files are valid

	// first, we must unpack the patch file, which is a zip
	HUNZIP huz;
	ZIPENTRY ze;
	DWORD numItems;

	UnzipOpenFile(&huz, patchFile.c_str(), 0);

	ze.Index = (DWORD) - 1;
	UnzipGetItem(huz, &ze);
	numItems = ze.Index;

	// unpack each file
	// we do not have to worry about changing directory as the compressed files
	// are in a directory with the same name as the patch file
	for(ze.Index = 0; ze.Index < numItems; ze.Index++) {
		UnzipGetItem(huz, &ze);
		UnzipItemToFile(huz, ze.Name, &ze);

		// now that we have the filename, patch it
		// we will have to strip the patch directory from the filename and the suffix
		// of the patch file (i.e. hoverrace-1.23.2/bin/HoverRace.exe.bsdiff ->
		//							bin/HoverRace.exe
		size_t bsdiffLoc = patchFile.rfind("bsdiff");
		if(bsdiffLoc == string::npos) {
			fprintf(stderr, "Invalid patchfile name %s\n", patchFile.c_str());
			return -1;
		} else {
			// strip patch directory
			size_t slashLoc = patchFile.find("/");
			if(slashLoc == string::npos) {
				fprintf(stderr, "Invalid patchfile dir %s\n", patchFile.c_str());
				return -1;
			} else {
				string fileToPatch = (patchFile.substr(0, bsdiffLoc)).substr(slashLoc + 1, string::npos);
				string newFile = fileToPatch + ".tmp"; // append .tmp for temporary file, we will move it later

				bsdiff_patch_file((char *) patchFile.c_str(), 
								  (char *) fileToPatch.c_str(),
								  (char *) newFile.c_str());

				// now move .tmp to original
				if(remove(fileToPatch.c_str()) != 0) {
					fprintf(stderr, "Error removing %s\n", fileToPatch.c_str());
					return -1;
				} else {
					if(rename(newFile.c_str(), fileToPatch.c_str()) != 0) {
						fprintf(stderr, "Error moving %s to %s\n", newFile.c_str(), fileToPatch.c_str());
						return -1;
					}
					// file has been patched successfully...
					// now we should update the statusbar but unfortunately it does not exist yet
				}
			}
		}
	}

	UnzipClose(huz);
}