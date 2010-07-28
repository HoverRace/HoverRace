
// GameDirector.h
// Interface for game client shells.
//
// Copyright (c) 2010 Michael Imamura.
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

#pragma once

#include "Control/Controller.h"

namespace HoverRace {
	namespace VideoServices {
		class VideoBuffer;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Interface for game client shells.
 * @author Michael Imamura
 */
class GameDirector
{
	public:
		virtual ~GameDirector() { }

		/**
		 * Request an orderly shutdown the of app.
		 * This call returns immediately; the shutdown will actually occur later,
		 * possibly when you least expect it.
		 * All normal "Are you sure?" confirmation prompts will be skipped.
		 */
		virtual void RequestShutdown() = 0;

		/**
		 * Notify this director that the IMR server configuration has changed.
		 */
		virtual void SignalServerHasChanged() = 0;

		/**
		 * Notify this director that the auto-update configuration has changed.
		 * @param newSetting The new value of the auto-update setting.
		 */
		virtual void ChangeAutoUpdates(bool newSetting) = 0;

		/**
		 * Notify this director that the video palette configuration has changed.
		 */
		virtual void AssignPalette() = 0;

		virtual VideoServices::VideoBuffer *GetVideoBuffer() const = 0;

		virtual Control::Controller *GetController() const = 0;

		/**
		 * Reload the control settings.
		 * @return The new control settings.
		 */
		virtual Control::Controller *ReloadController() = 0;

#	ifdef _WIN32
		virtual HWND GetWindowHandle() const = 0;
#	endif
};

}  // namespace HoverScript
}  // namespace Client
