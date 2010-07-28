
// VideoAudioPrefsPage.h
// Header for the "Video and Audio" preferences page.
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

#include "../../engine/Util/OS.h"

#include "PrefsPage.h"

namespace HoverRace {
namespace Client {

class GameDirector;

class VideoAudioPrefsPage : public PrefsPage
{
	typedef PrefsPage SUPER;
	public:
		VideoAudioPrefsPage(GameDirector *app);
		virtual ~VideoAudioPrefsPage();

	private:
		static const Util::OS::Monitor *GetSelectedMonitor(HWND hwnd,
			const Util::OS::monitors_t *monitors);
		static Util::OS::Resolution *GetSelectedResolution(HWND hwnd);
		static void UpdateResolutionList(HWND hwnd, Util::OS::monitors_t *monitors);
		static void InitMonitorList(HWND hwnd, Util::OS::monitors_t *monitors);

	protected:
		virtual BOOL DlgProc(HWND pWindow, UINT message, WPARAM wparam, LPARAM lparam);

	private:
		static void UpdateIntensityDialogLabels(HWND pWindow);
		static void UpdateAudioStatus(HWND hwnd);

	private:
		GameDirector *app;
};

}  // namespace Client
}  // namespace HoverRace
