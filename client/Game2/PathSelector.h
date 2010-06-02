
// PathSelector.h
// Header for the directory selection dialog.
//
// Copyright (c) 2009-2010 Michael Imamura.
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
 * File path selection dialog.
 * @author Michael Imamura
 */
class PathSelector
{
	public:
		PathSelector(const std::string &title);
		~PathSelector();

		bool ShowModal(HWND parent, Util::OS::path_t &path);

	protected:
		int DlgProc(HWND hwnd, UINT message, LPARAM lparam);
		static int CALLBACK DlgFunc(HWND hwnd, UINT message, LPARAM lparam, LPARAM data);

	private:
		std::wstring title;
		std::wstring initialPath;
};

}  // namespace Client
}  // namespace HoverRace
