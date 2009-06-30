
// PrefsPage.h
// Header for base class for preferences pages.
//
// Copyright (c) 2009 Michael Imamura.
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

namespace HoverRace {
namespace Client {

class PrefsPage
{
	public:
		PrefsPage(const std::string &title, DWORD dlgTmplId);
		virtual ~PrefsPage();

	public:
		void InitPropPage(PROPSHEETPAGEW &page, HINSTANCE hinst);

	private:
		virtual BOOL DlgProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) = 0;
		static BOOL CALLBACK DlgFunc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

	protected:
		HINSTANCE GetInstanceHandle() const { return hinst; }

	private:
		std::wstring title;
		DWORD dlgTmplId;
		HINSTANCE hinst;
		HWND hwnd;
};

}  // namespace Client
}  // namespace HoverRace
