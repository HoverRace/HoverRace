
// NetworkPrefsPage.h
// Header for the "Network" preferences page.
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

#include "PrefsPage.h"

class MR_GameApp;

namespace HoverRace {
namespace Client {

class NetworkPrefsPage : public PrefsPage
{
	typedef PrefsPage SUPER;
	public:
		NetworkPrefsPage(MR_GameApp *app);
		virtual ~NetworkPrefsPage();

	protected:
		virtual BOOL DlgProc(HWND pWindow, UINT message, WPARAM wparam, LPARAM lparam);

	private:
		MR_GameApp *app;
};

}  // namespace Client
}  // namespace HoverRace
