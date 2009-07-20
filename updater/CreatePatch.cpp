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

#include <boost/filesystem.hpp>

using namespace boost::filesystem;

int CreatePatch(string oldDir, string newDir, string patchFile) {
	// list all the files we will need to look at
	vector<string> dirListing = EnumerateDirectory(newDir);
}

/**
 * Return a string vector of all the files in the directory, searching recursively.
 */
vector<string> EnumerateDirectory(string directory) {
	vector<string> ret;
	
	recursive_directory_iterator it_end;
	recursive_directory_iterator it((path(directory)));
	
	while(it != it_end) {
		// check if file is directory or not
		// then do stuff with it
		if(is_regular(it->status())) {
			// we have a file... now we must add it to our list
			ret.push_back(it->path().file_string());
		}
	}

	return ret;
}