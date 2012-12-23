
// FolderViewer.cpp
// The folder viewer.
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

#include "StdAfx.h"

#include "../../engine/Util/OS.h"
#include "../../engine/Util/Str.h"

#include "FolderViewer.h"

using boost::format;
using boost::str;
using boost::wformat;

using namespace HoverRace::Util;

namespace fs = boost::filesystem;

namespace HoverRace {
namespace Client {

/**
 * Constructor.
 * @param path The path view (doesn't have to exist yet).
 */
FolderViewer::FolderViewer(const OS::path_t &path) :
	path(path)
{
}

/**
 * Open the OS-native folder viewer.
 * Note: Since this is the native viewer, this is not modal.
 * The parent window handle is for the alert dialogs and prompts.
 * @param parentWnd The parent window handle.
 */
void FolderViewer::Show(OS::wnd_t parentWnd)
{
	if (VerifyDirectory(parentWnd)) {
		OS::OpenPath(path);
	}
}

/**
 * Verify the directory exists.
 * If the directory does not exist, the user will be prompted to create it.
 * @param wnd The parent window for dialogs.
 * @param path The path to check.
 * @return @c true if the directory exists, @c false otherwise.
 */
bool FolderViewer::VerifyDirectory(OS::wnd_t parentWnd)
{
#ifdef _WIN32
	if (fs::exists(path)) {
		if (fs::is_directory(path)) {
			return true;
		} else {
			MessageBoxW(parentWnd,
				str(wformat(L"%s\n\n%s") %
					(const wchar_t*)Str::UW(_("The path is not a directory.")) %
					path).c_str(),
				PACKAGE_NAME_L,
				MB_ICONWARNING);
			return false;
		}
	} else {
		int resp = MessageBoxW(parentWnd,
			str(wformat(L"%s\n\n%s\n\n%s") %
				(const wchar_t*)Str::UW(_("Directory does not exist.")) %
				path %
				(const wchar_t*)Str::UW(_("Would you like to create it?"))).c_str(),
			PACKAGE_NAME_L,
			MB_ICONQUESTION | MB_YESNO);

		if (resp == IDYES) {
			fs::create_directories(path);
			if (fs::exists(path) && fs::is_directory(path)) {
				return true;
			} else {
				MessageBoxW(parentWnd,
					Str::UW(_("Failed to create directory.")),
					PACKAGE_NAME_L, MB_ICONWARNING);
				return false;
			}
		}

		return false;
	}
#else
	// Unimplemented.
	throw std::exception();
#endif
}

}  // namespace Client
}  // namespace HoverRace
