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

#include "boost/filesystem.hpp"

using namespace boost::filesystem;

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

	// find digest
	lstrcpy(ze.Name, "DIGEST");
	UnzipFindItem(huz, &ze, 0);
	UnzipItemToFile(huz, ze.Name, &ze);

	FILE *digest = fopen("DIGEST", "rb");

	if(digest == NULL) {
		fprintf(stderr, "Cannot open DIGEST!\n");
		return -1;
	}

	// unpack each file
	// we do not have to worry about changing directory as the compressed files
	// are in a directory with the same name as the patch file
	bool fail = false;
	while(!feof(digest) && !fail) {
		char digLine[1024];
		fgets(digLine, 1024, digest);

		if(feof(digest))
			break;

		// strip newline from end of string
		if(digLine[strlen(digLine) - 1] == 0x0A)
			digLine[strlen(digLine) - 1] = 0;

		char action = digLine[0];
		string filename = &digLine[2]; // first character is action, second char is space

		lstrcpy(ze.Name, filename.c_str());
		ze.Index = 0;
		if(UnzipFindItem(huz, &ze, 0) == ZR_NOTFOUND) {
			fprintf(stderr, "Not found in archive: %s\n", filename.c_str());
			break;
		}

		UnzipItemToFile(huz, ze.Name, &ze);

		switch(action) {
			case 'P': // patch file
				{
					string bsdiffPatch = filename;

					// now that	we have the filename, patch it
					// we will have to strip the patch directory from the filename and the suffix
					// of the patch file (i.e. hoverrace-1.23.2/bin/HoverRace.exe.bsdiff ->
					//							bin/HoverRace.exe
					size_t bsdiffLoc = bsdiffPatch.rfind(".bsdiff");
					if(bsdiffLoc == string::npos) {
						fprintf(stderr, "Invalid patchfile name %s\n", bsdiffPatch.c_str());
						fail = true;
					} else {
						// strip patch directory (it may not exist)
						size_t slashLoc = bsdiffPatch.find("/");

						string absPatchLoc = current_path().file_string() + "\\" + bsdiffPatch;
						string fileToPatch = targetDir + (bsdiffPatch.substr(0, bsdiffLoc));
						string newFile = fileToPatch + ".tmp"; // append .tmp for temporary file, we will move it later

						bsdiff_patch_file((char *) absPatchLoc.c_str(),
										  (char *) fileToPatch.c_str(),
										  (char *) newFile.c_str());

						// now move .tmp to original
						if(remove(fileToPatch.c_str()) != 0) {
							fprintf(stderr, "Error removing %s\n", fileToPatch.c_str());
							fail = true;
						} else {
							if(rename(newFile.c_str(), fileToPatch.c_str()) != 0) {
								fprintf(stderr, "Error moving %s to %s\n", newFile.c_str(), fileToPatch.c_str());
								fail = true;
							}
							
							// file has been patched successfully...
							// now we should update the statusbar but unfortunately it does not exist yet
							fprintf(stdout, "File %s patched\n", fileToPatch.c_str());
						}
					}
				}
				break;

			case 'A': // add file
				{
					string oldFile = current_path().file_string() + "\\" + filename;
					string newFile = targetDir + "\\" + filename;

					if(exists(newFile)) {
						fprintf(stderr, "File %s already exists!\n", newFile.c_str());
						fail = true;
					} else {
						rename(path(oldFile), path(newFile));
						fprintf(stdout, "File %s added\n", filename.c_str());
					}

					break;
				}

			case 'D': // delete file
				remove(path(targetDir + "\\" + filename));
				fprintf(stdout, "File %s removed\n", filename);
				break;
		}

		if(action != 'D') {
			// remove extracted file
			remove(path(current_path().file_string() + "\\" + filename));

			if(strcmp(path(filename).branch_path().file_string().c_str(), "") != 0) // must remove parent directory if it exists
				remove_all(path(filename).branch_path());
		}
	}

	// close and remove digest
	fclose(digest);
	remove(path(current_path().file_string() + "\\DIGEST"));

	UnzipClose(huz);

	if(!fail)
		fprintf(stdout, "Patching complete!\n");
}