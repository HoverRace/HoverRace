
// GameDirector.h
// Interface for game client shells.
//
// Copyright (c) 2010, 2013 Michael Imamura.
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

namespace HoverRace {
	namespace Client {
		class ClientSession;
		class Rules;
		class Scene;
		typedef std::shared_ptr<Scene> ScenePtr;
	}
	namespace Control {
		class InputEventController;
	}
	namespace Display {
		class Display;
	}
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
		 * Push a new scene to the foreground.
		 * @note This may be called from any thread.
		 * @param scene The scene to push.
		 */
		virtual void RequestPushScene(const ScenePtr &scene) = 0;

		/**
		 * Return to the previous scene, if any.
		 * @note This may be called from any thread.
		 */
		virtual void RequestPopScene() = 0;

		/**
		 * Replace the current scene and all background scenes with a new
		 * foreground scene.
		 * @note This may be called from any thread.
		 * @param scene The scene to push.
		 */
		virtual void RequestReplaceScene(const ScenePtr &scene) = 0;

		/**
		 * Request a return to the main menu.
		 */
		virtual void RequestMainMenu() = 0;

		/**
		 * Request a new local practice session.
		 * @param rules The settings for the session.
		 */
		virtual void RequestNewPracticeSession(std::shared_ptr<Rules> rules) = 0;

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

		virtual Display::Display *GetDisplay() const = 0;

		virtual VideoServices::VideoBuffer *GetVideoBuffer() const = 0;

		virtual Control::InputEventController *GetController() const = 0;

		/**
		 * Reload the control settings.
		 * @return The new control settings.
		 */
		virtual Control::InputEventController *ReloadController() = 0;

		typedef boost::signals2::signal<void(ClientSession*)> sessionChangedSignal_t;

		/**
		 * Access the signal that indicates that the gameplay session
		 * started or stopped.
		 *
		 * Subscribers to this signal will get the ClientSession instance
		 * if the session is starting or @c nullptr if the session has
		 * ended.
		 */
		virtual sessionChangedSignal_t &GetSessionChangedSignal() = 0;


#	ifdef _WIN32
		virtual HWND GetWindowHandle() const = 0;
#	endif
};

}  // namespace HoverScript
}  // namespace Client
