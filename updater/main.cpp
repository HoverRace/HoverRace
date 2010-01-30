// HoverRace auto-updater
// main.cpp
//
// This file defines the command-line automatic updater for HoverRace
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
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
	#include <io.h>
#else
	#include <unistd.h>
#endif

#include <boost/filesystem.hpp>
#include <windows.h>

#include "bsdiff/bspatch.h"
#include "bsdiff/bsdiff.h"

// for zip/unzip of patches
#include <LiteUnzip.h>
#include <LiteZip.h>

#define new DEBUG_NEW

using namespace std;
using namespace boost::filesystem;

bool dirExists(string dir); /// check if a directory exists; auxiliary function
bool fileExists(string dir); /// check if a file exists; auxiliary function
void waitExit(int code);

vector<string> EnumerateDirectory(string dir);

int main(int argc, const char **argv) {
	printf("HoverRace automatic updater and patch generator\n");
	//Sleep(25000); // time to attach

	bool createUpdate = false; // if createUpdate is false we are updating
	string targetDir = "";
	string sourceDir = "";
	string patchFile = "";

	// Windows has no getopt() therefore we must do it by hand and as a result my implementation is not the best or the
	// cleanest but it does work
	printf("argc %d\n", argc);
	for(int i = 1; i < argc; i++) {
		printf("next arg is %s\n", argv[i]);
		printf("after that is %s\n", argv[i + 1]);
		if(argv[i][0] == '-') {
			if(argv[i][1] == 'c') {
				createUpdate = true;

				if(i == argc - 1) {
					fprintf(stderr, "No updated version path given!\n");
					waitExit(1);
				} else {
					sourceDir = argv[++i];
				}
			} else if(argv[i][1] == 'h') {
				fprintf(stderr, "Usage: %s [-c /path/to/updated/version/] /path/to/hoverrace/ patchfile.zip\n", argv[0]);
				fprintf(stderr, "  The -c option specifies that an update should be created\n");
				waitExit(1);
			} else {
				// no gettext here... this should not ever be used by users
				fprintf(stderr, "Unknown option %s\n", argv[i]);
				waitExit(1);
			}
		} else {
			// must be our working directory
			targetDir = argv[i];

			if(i == argc - 1) { // missing an argument
				fprintf(stderr, "No bsdiff zip file supplied!\n");
				waitExit(1);
			} else {
				printf("going to set our patch file to %s\n", argv[i + 1]);
				patchFile = argv[i + 1];
				printf("patchFile is %s\n", patchFile.c_str());
			}

			break;
		}
	}

	if(createUpdate && sourceDir == "") {
		fprintf(stderr, "No source directory supplied!\n");
		waitExit(1);
	} 

	if(targetDir == "") {
		fprintf(stderr, "No target directory supplied!\n");
		waitExit(1);
	}

	// check that updated directory is valid
	if(createUpdate && !is_directory(sourceDir)) {
		fprintf(stderr, "Source directory %s does not exist!\n", sourceDir.c_str());
		waitExit(1);
	}

	// check that directory is valid
	if(!is_directory(targetDir)) {
		fprintf(stderr, "Target directory %s does not exist!\n", targetDir.c_str());
		waitExit(1);
	}

	// now that all the setup is done, begin the actual work
	if(createUpdate) {
		// diff source and target
		printf("Creating patch from %s to %s\n", sourceDir, targetDir);
		//return (CreatePatch(targetDir, sourceDir, patchFile));

		/***********************************
		 * create a patch                  *
		 ***********************************/
		{
			// list all the files we will need to look at
			printf("Enumerating files...\n");

			vector<string> dirListing = EnumerateDirectory(sourceDir);
			vector<string> oldListing = EnumerateDirectory(targetDir);

			// set up vectors for the possible actions needed to take on each file
			// the vectors will hold the filenames
			vector<string> patchFiles;
			vector<string> newFiles; // need to be added

			// create directory for temporary files
			// if this directory already exists we throw an error and exit
			if(is_directory("temp_patch")) {
				fprintf(stdout, "Directory temp_patch exists!  Exiting before bad things happen.  Remove directory and try again...\n");
				return -1;
			}

			create_directory("temp_patch");

			// now we must create an individual patchfile for each listed file we must diff
			for(vector<string>::iterator it = dirListing.begin(); it != dirListing.end(); it++) {
				printf("Creating patch for %s\n", (*it).c_str());

				// see if this file exists in the old directory structure
				vector<string>::iterator index = find(oldListing.begin(), oldListing.end(), *it);
				if(index == oldListing.end()) {
					// file does not exist in old directory
					// so we have to copy the file over directly
					string oldFile = path(targetDir).file_string() + "\\" + *it;
					string patchName = "\\temp_patch\\" + *it; // same filename
					string newFile = current_path().file_string() + patchName;

					copy_file(path(oldFile), path(newFile));

					newFiles.push_back(*it);
				} else {
					// we must create a patch
					string oldFile = path(sourceDir).file_string() + "/" + *index;
					string newFile = path(targetDir).file_string() + "/" + *it;
					// separate the actual filename to construct the patch filename and add '.bsdiff'
					string patchName = *it + ".bsdiff";
					string curPatchFile = current_path().file_string() + "/temp_patch/" + patchName;

					// remove from old listing vector (not necessary to remove from new listing vector)
					index = oldListing.erase(index);

					// ensure directory exists
					if(!exists("temp_patch/" + path(patchName).branch_path().file_string()))
						create_directory("temp_patch/" + path(patchName).branch_path().file_string());

					// now create the patch
					bsdiff_create_patch((char *) curPatchFile.c_str(), (char *) oldFile.c_str(), (char *) newFile.c_str());

					// verify patch file exists
					if(!exists(curPatchFile)) {
						fprintf(stderr, "Error creating patch file %s! Exiting...\n", patchFile.c_str());
						return -1;
					}

					// with the patch created we must add to our list of files that need patching
					patchFiles.push_back(patchName);
				}
			}

			// now, what is left in our old listing that we haven't removed, we will need to erase
			string digestFile = current_path().file_string() + "/temp_patch/DIGEST";
			FILE *digest = fopen(digestFile.c_str(), "wb");

			// write files to patch to digest
			for(vector<string>::iterator it = patchFiles.begin(); it != patchFiles.end(); it++) {
				// erase any leading slashes
				if((*it).at(0) == '\\' || (*it).at(0) == '/')
					(*it).erase(0, 1);
				fprintf(digest, "P %s\n", (*it).c_str());
			}

			// write files to add to digest
			for(vector<string>::iterator it = newFiles.begin(); it != newFiles.end(); it++) {
				if((*it).at(0) == '\\' || (*it).at(0) == '/')
					(*it).erase(0, 1);
				fprintf(digest, "A %s\n", (*it).c_str());
			}

			// write files to remove to digest
			for(vector<string>::iterator it = oldListing.begin(); it != oldListing.end(); it++) {
				if((*it).at(0) == '\\' || (*it).at(0) == '/')
					(*it).erase(0, 1);
				fprintf(digest, "D %s\n", (*it).c_str());
			}

			fclose(digest);

			// now, we zip everything up
			HZIP hz;

			ZipCreateFile(&hz, patchFile.c_str(), 0);

			vector<string> filesToZip = EnumerateDirectory(current_path().file_string() + "/temp_patch/");

			for(vector<string>::iterator it = filesToZip.begin(); it != filesToZip.end(); it++) {
				int ret = ZipAddFile(hz, (*it).c_str(), ("temp_patch/" + *it).c_str());
				if(ret) {
					char msg[1000];
					ZipFormatMessage(ret, msg, sizeof(msg));
					printf("Error zipping file %s: %s\n", (*it).c_str(), msg);
				}
			}

			ZipClose(hz);

			// remove all our temporary files
			remove_all("temp_patch");

			return 0;
		}
	} else {
		/***********************************
		 * apply a patch                   *
		 ***********************************/

		// check that patch exists
		if(!exists(patchFile)) {
			fprintf(stderr, "Patch file %s does not exist!\n", patchFile.c_str());
			waitExit(1);
		}

		// check that HoverRace is not running
		HWND hrWin = FindWindowW(L"HoverRaceClass", NULL);

		if(hrWin != NULL) {
			// kill it, with user confirmation
			printf("HoverRace is running, we must kill it first.\n");
			printf("For now just letting it run...\n");
			//SendMessage(hrWin, WM_CLOSE, 0, 0);
			// if we wanted to kill instantly we would use WM_DESTROY
		}

		// now, HoverRace is closed; do the patching
		printf("Will patch with file %s\n", patchFile.c_str());
		//PatchHoverRace(targetDir, patchFile);
		{
			// we assume that the files are valid
			printf("Beginning patch process...\n");

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

			printf("Opening DIGEST...\n");

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

				if(filename.at(0) == '\\' || filename.at(0) == '/') // strip leading slash
					filename.erase(0, 1);

				//printf("Searching for %s\n", filename.c_str());
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
								string fileToPatch = targetDir + "/" + (bsdiffPatch.substr(0, bsdiffLoc));
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
	}

	waitExit(0);
}

void waitExit(int code) {
	Sleep(4000); // wait for four seconds
	exit(code);
}

/**
 * Return a string vector of all the files in the directory, searching recursively.
 */
vector<string> EnumerateDirectory(string directory) {
	vector<string> ret;
	
	recursive_directory_iterator it_end;
	recursive_directory_iterator it((path(directory)));
	
	string dir = string(path(directory).file_string());
	int dirlen = dir.length();

	while(it != it_end) {
		// check if file is directory or not
		// then do stuff with it
		if(is_regular(it->status())) {
			// we have a file... now we must add it to our list
			string item = it->path().file_string();
			// remove directory from path
			ret.push_back(item.substr(dirlen, string::npos));
		}

		it++;
	}

	return ret;
}