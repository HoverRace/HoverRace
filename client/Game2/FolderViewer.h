
// FolderViewer.h
// Header for the folder viewer.
//
// Copyright (c) 2012 Michael Imamura.
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

#pragma once

#include "../../engine/Util/OS.h"

namespace HoverRace {
namespace Client {

/**
 * Opens a filesytem folder for viewing, using the OS-specific folder viewer.
 * This is a wrapper around OS::OpenPath with extra checks to make sure the
 * path is actually a directory (and inform the user if it is not).  If the
 * path does not exist, the user is prompted to create it.
 * @author Michael Imamura
 */
class FolderViewer
{
	public:
		FolderViewer(const Util::OS::path_t &path);
		~FolderViewer() { }

	public:
		void Show(Util::OS::wnd_t parentWnd);

	private:
		bool VerifyDirectory(Util::OS::wnd_t parentWnd);

	private:
		const Util::OS::path_t path;
};

}  // namespace Client
}  // namespace HoverRace
