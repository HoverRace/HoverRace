// HoverRace Auto-Updater
// CreatePatch.cpp
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

#include "CreatePatch.h"

#include <algorithm>
#include <boost/filesystem.hpp>
#include "bsdiff/bsdiff.h"
#include "LiteZip.h"

using namespace boost::filesystem;

int CreatePatch(string oldDir, string newDir, string patchFile) {
	// list all the files we will need to look at
	vector<string> dirListing = EnumerateDirectory(newDir);
	vector<string> oldListing = EnumerateDirectory(oldDir);

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
			string oldFile = path(newDir).file_string() + "\\" + *it;
			string patchName = "\\temp_patch\\" + *it; // same filename
			string newFile = current_path().file_string() + patchName;

			copy_file(path(oldFile), path(newFile));

			newFiles.push_back(patchName);
		} else {
			string oldFile = path(oldDir).file_string() + "/" + *index;
			string newFile = path(newDir).file_string() + "/" + *it;
			// separate the actual filename to construct the patch filename and add '.bsdiff'
			string patchName = path(newFile).leaf() + ".bsdiff";
			string curPatchFile = current_path().file_string() + "/temp_patch/" + patchName;

			// remove from vector
			index = oldListing.erase(index);
			//it = dirListing.erase(it);

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
	FILE *digest = fopen(digestFile.c_str(), "w");

	// write files to patch to digest
	for(vector<string>::iterator it = patchFiles.begin(); it != patchFiles.end(); it++)
		fprintf(digest, "PAT %s\n", (*it).c_str());

	// write files to add to digest
	for(vector<string>::iterator it = newFiles.begin(); it != newFiles.end(); it++)
		fprintf(digest, "ADD %s\n", (*it).c_str());

	// write files to remove to digest
	for(vector<string>::iterator it = oldListing.begin(); it != oldListing.end(); it++)
		fprintf(digest, "DEL %s\n", (*it).c_str());

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