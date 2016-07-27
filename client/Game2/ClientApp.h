
// ClientApp.h
//
// Copyright (c) 2010, 2014-2016 Michael Imamura.
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

#include "../../engine/Util/OS.h"

#include "Observer.h"

#include "GameDirector.h"

namespace HoverRace {
	namespace Client {
		namespace HoverScript {
			class ConsoleScene;
			class DebugPeer;
			class GamePeer;
			class HighConsole;
			class InputPeer;
			class SessionPeer;
			class SysConsole;
			class SysEnv;
		}
		class Announcement;
		class DebugScene;
		class HighObserver;
		class LoadingScene;
		class Rulebook;
		class RulebookLibrary;
		class Scene;
		class StatusOverlayScene;
	}
	namespace Control {
		class InputEventController;
	}
	namespace Display {
		class ActiveText;
		class Display;
	}
	namespace Player {
		class AvatarGallery;
	}
	namespace Script {
		class Core;
	}
	namespace Util {
		class Profiler;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Main game shell.
 * @author Michael Imamura
 */
class ClientApp : public GameDirector
{
	using SUPER = GameDirector;

public:
	ClientApp();
	virtual ~ClientApp();

private:
	void LoadInitialProfile();

private:
	std::string GetWindowTitle();
	void OnWindowResize(int w, int h);
	void IncFrameCount();
	void AdvanceScenes(Util::OS::timestamp_t tick);
	void PrepareScenes();
	void RenderScenes();
	void RenderFrame();

public:
	enum class ExitMode
	{
		QUITTING,  ///< Normal exit.
		SOFT_RESTART,  ///< Create and run a new ClientApp instance.
	};
	ExitMode MainLoop();

private:
	void OnConsoleToggle();

private:
	using sceneStack_t = std::list<std::shared_ptr<Scene>>;
	void SetForegroundScene();
	void SetForegroundScene(const std::shared_ptr<Scene> &iter);
	void PushScene(const std::shared_ptr<Scene> &scene);
	void PopScene();
	void ReplaceScene(const std::shared_ptr<Scene> &scene);
	void TerminateAllScenes();

public:
	// GameDirector
	Scene *GetForegroundScene() const override { return sceneStack.empty() ? nullptr : sceneStack.back().get(); }
	void RequestPushScene(const std::shared_ptr<Scene> &scene) override;
	void RequestPopScene() override;
	void RequestReplaceScene(const std::shared_ptr<Scene> &scene) override;
	void RequestMainMenu(
		std::shared_ptr<LoadingScene> loadingScene =
			std::shared_ptr<LoadingScene>()) override;
	void RequestStatusPresentation() override;
	void RequestNewPracticeSession(
		std::shared_ptr<Rules> rules,
		std::shared_ptr<LoadingScene> loadingScene =
			std::shared_ptr<LoadingScene>()) override;
	void RequestAnnouncement(std::shared_ptr<Announcement> ann) override;
	void RequestShutdown() override;
	void RequestSoftRestart() override;
	Display::Display *GetDisplay() const override { return display.get(); }
	VideoServices::VideoBuffer *GetVideoBuffer() const override;
	Control::InputEventController *GetController() const override { return controller.get(); }
	Control::InputEventController *ReloadController() override;
	std::shared_ptr<Player::AvatarGallery> ShareAvatarGallery() const override { return avatarGallery; }
	Roster *GetParty() const override { return party.get(); }
	std::shared_ptr<Player::Player> ShareUiPilot() const override;
	sessionChangedSignal_t &GetSessionChangedSignal() override { return sessionChangedSignal; }

private:
	bool needsDevWarning;  ///< Display dev release warning on next menu.
	bool needsLocaleCheck;  ///< Check locale and warn on next menu.
	MR_UInt32 userEventId;

	std::unique_ptr<Display::Display> display;

	std::unique_ptr<Control::InputEventController> controller;
	std::shared_ptr<Player::AvatarGallery> avatarGallery;
	std::unique_ptr<Roster> party;

	std::unique_ptr<Script::Core> scripting;
	std::unique_ptr<RulebookLibrary> rulebookLibrary;
	std::unique_ptr<HoverScript::DebugPeer> debugPeer;
	std::unique_ptr<HoverScript::GamePeer> gamePeer;
	std::unique_ptr<HoverScript::InputPeer> inputPeer;
	std::unique_ptr<HoverScript::SysEnv> sysEnv;
	std::unique_ptr<HoverScript::SysConsole> sysConsole;

	// Stats counters.
	std::unique_ptr<Display::ActiveText> fpsLbl;
	unsigned int frameCount;
	Util::OS::timestamp_t lastTimestamp;
	double fps;
	const bool &showFps;

	sceneStack_t sceneStack;
	std::shared_ptr<Scene> fgScene;  ///< The scene that currently has input focus.
	std::unique_ptr<StatusOverlayScene> statusOverlayScene;
	bool showOverlay;
	std::unique_ptr<DebugScene> debugScene;
	const bool &showDebug;
	std::list<std::shared_ptr<Announcement>> announcements;

	std::weak_ptr<HoverScript::ConsoleScene> consoleScene;

	sessionChangedSignal_t sessionChangedSignal;

	boost::signals2::connection consoleToggleConn;

	std::shared_ptr<Util::Profiler> rootProfiler;
	std::shared_ptr<Util::Profiler> advanceProfiler;
	std::shared_ptr<Util::Profiler> prepareProfiler;
	std::shared_ptr<Util::Profiler> renderProfiler;
};

}  // namespace HoverScript
}  // namespace Client
