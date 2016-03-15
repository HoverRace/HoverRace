
// ClientApp.cpp
//
// Copyright (c) 2010, 2013-2016 Michael Imamura.
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

#include <SDL2/SDL.h>

#include "../../engine/Exception.h"
#include "../../engine/Control/Controller.h"
#include "../../engine/Display/ActiveText.h"
#include "../../engine/Display/UiLayoutFlags.h"
#include "../../engine/Display/SDL/SdlDisplay.h"
#include "../../engine/MainCharacter/MainCharacter.h"
#include "../../engine/Model/Track.h"
#include "../../engine/Parcel/TrackBundle.h"
#include "../../engine/Player/DemoProfile.h"
#include "../../engine/Player/LocalPlayer.h"
#include "../../engine/Player/LocalProfile.h"
#include "../../engine/Player/ProfileGallery.h"
#include "../../engine/Util/Config.h"
#include "../../engine/Util/Loader.h"
#include "../../engine/Util/Locale.h"
#include "../../engine/Util/Profiler.h"
#include "../../engine/Util/Str.h"
#include "../../engine/VideoServices/VideoBuffer.h"

#include "HoverScript/ClientScriptCore.h"
#include "HoverScript/ConsoleScene.h"
#include "HoverScript/DebugPeer.h"
#include "HoverScript/GamePeer.h"
#include "HoverScript/InputPeer.h"
#include "HoverScript/SessionPeer.h"
#include "HoverScript/StyleEnv.h"
#include "HoverScript/SysConsole.h"
#include "HoverScript/SysEnv.h"
#include "ClientSession.h"
#include "DebugScene.h"
#include "DemoGameScene.h"
#include "LoadingScene.h"
#include "MainMenuScene.h"
#include "MessageScene.h"
#include "PlayGameScene.h"
#include "Roster.h"
#include "Rulebook.h"
#include "RulebookLibrary.h"
#include "Rules.h"
#include "Scene.h"
#include "StatusOverlayScene.h"
#include "TestLabScene.h"

#ifdef _WIN32
#	include "resource.h"
#endif

#include "ClientApp.h"

using namespace HoverRace::Client::HoverScript;
using namespace HoverRace::Util;
namespace SoundServer = HoverRace::VideoServices::SoundServer;
using HoverRace::Control::InputEventController;

namespace HoverRace {
namespace Client {

namespace {

// SDL user event codes.
enum {
	REQ_EVT_SCENE_PUSH,
	REQ_EVT_SCENE_POP,
	REQ_EVT_SCENE_REPLACE,
	REQ_EVT_ANN_ADD,
	REQ_EVT_SOFT_RESTART,
};

/**
 * Used by @c REQ_EVT_SCENE_PUSH and @c REQ_SCENE_REPLACE to transfer
 * the requested scene.
 */
struct SceneHolder
{
	SceneHolder(std::shared_ptr<Scene> scene) : scene(std::move(scene)) { }
	std::shared_ptr<Scene> scene;
};

/**
 * Used by @c REQ_EVT_ANN_ADD to transfer the requested announcement.
 */
struct AnnouncementHolder
{
	AnnouncementHolder(std::shared_ptr<Announcement> ann) :
		ann(std::move(ann)) { }
	std::shared_ptr<Announcement> ann;
};

/// A new local profile using the current logged-in user.
class NewProfile : public Player::LocalProfile
{
	using SUPER = Player::LocalProfile;

public:
	NewProfile() : SUPER()
	{
		SetName(OS::GetUsername());
	}
};

}  // namespace

ClientApp::ClientApp() :
	SUPER(),
	needsDevWarning(false), needsLocaleCheck(true),
	userEventId(SDL_RegisterEvents(1)),
	controller(new InputEventController()),
	party(new Roster()),
	fpsLbl(), frameCount(0), lastTimestamp(0), fps(0.0),
	showFps(Config::GetInstance()->runtime.showFramerate),
	sceneStack(), fgScene(),
	statusOverlayScene(), showOverlay(false),
	debugScene(), showDebug(Config::GetInstance()->runtime.enableDebugOverlay),
	rootProfiler(std::make_shared<Profiler>("ROOT")),
	advanceProfiler(rootProfiler->AddSub("advance")),
	prepareProfiler(rootProfiler->AddSub("prepare")),
	renderProfiler(rootProfiler->AddSub("render"))
{
	auto cfg = Config::GetInstance();

	LoadInitialProfile();

	// Create the system console and execute the initialization scripts.
	// This allows the script to modify the configuration (e.g. for unit tests).
	scripting.reset((new ClientScriptCore())->Reset());
	rulebookLibrary.reset(new RulebookLibrary(scripting.get()));
	rulebookLibrary->Reload();
	debugPeer.reset(new DebugPeer(*scripting, *this));
	gamePeer.reset(new GamePeer(*scripting, *this, *rulebookLibrary));
	inputPeer.reset(new InputPeer(*scripting, *controller));
	sysEnv.reset(new SysEnv(*scripting, *debugPeer, *gamePeer, *inputPeer));
	for (OS::path_t &initScript : cfg->runtime.initScripts) {
		if (!initScript.empty()) {
			sysEnv->RunScript(initScript);
		}
	}
	sysConsole.reset(new SysConsole(*scripting, *this,
		*debugPeer, *gamePeer, *inputPeer));

	// Create the main window and SDL surface.
	display.reset(new Display::SDL::SdlDisplay(GetWindowTitle()));
	SDL_DisplayMode desktopMode;
	int monitorIdx = cfg->video.fullscreen ?
		cfg->video.fullscreenMonitorIndex : 0;
	if (SDL_GetCurrentDisplayMode(monitorIdx, &desktopMode) < 0) {
		throw Exception(SDL_GetError());
	}
	display->OnDesktopModeChanged(desktopMode.w, desktopMode.h);

	// Run the stylesheet, if it exists.
	auto stylesheetPath = cfg->GetMediaPath();
	stylesheetPath /= Str::UP("themes");
	stylesheetPath /= Str::UP("default");
	StyleEnv(*scripting, *display, stylesheetPath).RunStylesheet();

	gamePeer->SetDisplay(display.get());

	namespace LayoutFlags = Display::UiLayoutFlags;
	fpsLbl.reset(new Display::ActiveText("FPS:",
		Display::UiFont(20, Display::UiFont::BOLD),
		Display::Color(0xff, 0xff, 0x7f, 0x00),
		LayoutFlags::UNSCALED | LayoutFlags::FLOATING));
	fpsLbl->AttachView(*display);

	// SDL2 automatically initiates text input mode.
	// We want to have each scene decide whether they expect text input or not.
	SDL_StopTextInput();

	// Save the config in case we set any initial options,
	// such as the default profile.
	cfg->Save();
}

ClientApp::~ClientApp()
{
	gamePeer->SetDisplay(nullptr);
}

/// Attempt to load the default profile to the roster.
void ClientApp::LoadInitialProfile()
{
	Player::ProfileGallery gallery;
	auto cfg = Config::GetInstance();
	std::shared_ptr<Player::Profile> profile;

	// If there are no profiles, then we'll create a fresh new one and set
	// that as the default.
	if (gallery.IsEmpty()) {
		HR_LOG(info) << "No known profiles; creating new default profile.";
		profile = std::make_shared<NewProfile>();
		profile->Edit()->Save();

		cfg->player.defaultProfile = profile->GetUidStr();

		HR_LOG(info) << "New default profile: " << profile->GetUidStr();
	}
	else {
		// Attempt to load the default profile.
		// If there's no default profile or the profile couldn't be loaded, we
		// use a demo profile.
		//TODO: Instead of demo profile, trigger profile selection scene for later.

		if (cfg->player.defaultProfile.empty()) {
			// No default profile set; use the first local profile we can find.
			// (We currently assume that all gallery profiles are local,
			// and since we already checked if the gallery is empty, we can
			// just use the first one).
			profile = *(gallery.begin());
		}
		else {
			profile = gallery.FindUid(cfg->player.defaultProfile);
		}

		if (!profile) {
			HR_LOG(info) << "Default profile does not exist, using demo: " <<
				cfg->player.defaultProfile;
			profile = std::make_shared<Player::DemoProfile>();
		}
	}

	party->AddPlayer(
		std::make_shared<Player::LocalPlayer>(profile, true, true));
}

std::string ClientApp::GetWindowTitle()
{
	Config *cfg = Config::GetInstance();

	std::ostringstream oss;
	oss << PACKAGE_NAME " " << cfg->GetVersion();
	if (cfg->IsPrerelease()) {
		oss << " (" << _("Version", "testing") << ')';
	}
	if (SoundServer::IsDisabled()) {
		oss << " (" << _("Silent Mode") << ')';
	}

	return oss.str();
}

/**
 * Handle when the window is resized.
 * @param w The new width.
 * @param h The new height.
 */
void ClientApp::OnWindowResize(int w, int h)
{
	auto &vidCfg = Config::GetInstance()->video;
	vidCfg.xRes = w;
	vidCfg.yRes = h;

	display->OnDisplayConfigChanged();
}

/**
 * Increment the frame counter for stats purposes.
 * This should be called once per frame.
 */
void ClientApp::IncFrameCount()
{
	static boost::format fpsFmt("FPS: %0.2f");

	// Don't start counting until the first frame.
	if (lastTimestamp == 0) lastTimestamp = OS::Time();

	++frameCount;
	OS::timestamp_t curTimestamp = OS::Time();
	OS::timestamp_t diff = OS::TimeDiff(curTimestamp, lastTimestamp);

	if (diff > 1000) {
		fps = static_cast<double>(frameCount) /
			static_cast<double>(diff / 1000);
		lastTimestamp = curTimestamp;
		frameCount = 0;

		if (Config::GetInstance()->runtime.showFramerate) {
			fpsLbl->SetText(boost::str(fpsFmt % fps));
		}
	}
}

void ClientApp::AdvanceScenes(Util::OS::timestamp_t tick)
{
	Profiler::Sampler sampler(*advanceProfiler);

	auto iter = sceneStack.begin();
	while (iter != sceneStack.end()) {
		Scene &scene = **iter;
		Scene::Phase phase = scene.GetPhase();
		if (phase != Scene::Phase::INITIALIZING &&
			phase != Scene::Phase::STOPPED)
		{
			scene.AdvanceScene(tick);
		}

		// If the scene has entered the STOPPED state, it's deletable.
		if (scene.GetPhase() == Scene::Phase::STOPPED) {
			iter = sceneStack.erase(iter);
		}
		else {
			++iter;
		}
	}

	if (showOverlay) statusOverlayScene->AdvanceScene(tick);
	if (showDebug) debugScene->AdvanceScene(tick);
}

void ClientApp::PrepareScenes()
{
	Profiler::Sampler sampler(*prepareProfiler);

	for (const auto &scene : sceneStack) {
		scene->PrepareScene();
	}
	if (showOverlay) statusOverlayScene->PrepareScene();
	if (showDebug) debugScene->PrepareScene();
	if (showFps) fpsLbl->PrepareRender();
}

void ClientApp::RenderScenes()
{
	Profiler::Sampler sampler(*renderProfiler);

	for (const auto &scene : sceneStack) {
		Scene::Phase phase = scene->GetPhase();
		if (phase != Scene::Phase::INITIALIZING &&
			phase != Scene::Phase::STOPPED)
		{
			scene->RenderScene();
		}
	}
	if (showOverlay) statusOverlayScene->RenderScene();
	if (showDebug) debugScene->RenderScene();
	if (showFps) fpsLbl->Render();
}

void ClientApp::RenderFrame()
{
	IncFrameCount();

	if (sceneStack.empty()) {
		VideoServices::VideoBuffer::Lock lock(display->GetLegacyDisplay());
		display->GetLegacyDisplay().Clear();
	}
	else {
		PrepareScenes();
		RenderScenes();
	}

	display->Flip();
}

ClientApp::ExitMode ClientApp::MainLoop()
{
	bool quit = false;
	ExitMode retv = ExitMode::QUITTING;
	SDL_Event evt;

	statusOverlayScene.reset(new StatusOverlayScene(*display, *this));
	debugScene.reset(new DebugScene(*display, *this));

	const auto &runtimeCfg = Config::GetInstance()->runtime;
	needsDevWarning =
		!runtimeCfg.skipStartupWarning &&
		runtimeCfg.initScripts.empty();

	RequestMainMenu();

	// Fire all on_init handlers.
	gamePeer->OnInit();

	while (!quit) {
		Profiler::Sampler sampler(*rootProfiler);

		OS::timestamp_t tick = OS::Time();

		while (SDL_PollEvent(&evt) && !quit) {
			if (evt.type >= SDL_KEYDOWN && evt.type <= SDL_MULTIGESTURE) {
				// Input events are routed to the InputEventController.
				controller->ProcessInputEvent(evt);
				continue;
			}
			if (evt.type == userEventId) {
				switch (evt.user.code) {
					case REQ_EVT_SCENE_PUSH: {
						SceneHolder *holder = static_cast<SceneHolder*>(evt.user.data1);
						PushScene(holder->scene);
						delete holder;
						break;
					}

					case REQ_EVT_SCENE_POP:
						PopScene();
						break;

					case REQ_EVT_SCENE_REPLACE: {
						SceneHolder *holder = static_cast<SceneHolder*>(evt.user.data1);
						ReplaceScene(holder->scene);
						delete holder;
						break;
					}

					case REQ_EVT_ANN_ADD: {
						AnnouncementHolder *holder = static_cast<AnnouncementHolder*>(evt.user.data1);
						statusOverlayScene->Announce(holder->ann);
						delete holder;
						break;
					}

					case REQ_EVT_SOFT_RESTART:
						quit = true;
						retv = ExitMode::SOFT_RESTART;
						break;
				}
			}
			else {
				switch (evt.type) {
					case SDL_QUIT:
						quit = true;
						break;

					case SDL_WINDOWEVENT:
						switch (evt.window.event) {
							case SDL_WINDOWEVENT_RESIZED:
								OnWindowResize(evt.window.data1, evt.window.data2);
								break;
						}
						break;
				}
			}
		}
		if (quit) break;

		AdvanceScenes(tick);
		RenderFrame();

		if (runtimeCfg.profiling && frameCount == 0) {
			rootProfiler->Lap();
			HR_LOG(info) << rootProfiler->GetName() << "  " << rootProfiler->GetLastLap();
			HR_LOG(info) << "  " << advanceProfiler->GetName() << "  " << advanceProfiler->GetLastLap();
			HR_LOG(info) << "  " << prepareProfiler->GetName() << "  " << prepareProfiler->GetLastLap();
			HR_LOG(info) << "  " << renderProfiler->GetName() << "  " << renderProfiler->GetLastLap();
			HR_LOG(info) << "  other  " << rootProfiler->GetOtherTime();
		}
	}

	TerminateAllScenes();
	statusOverlayScene.reset();

	gamePeer->OnShutdown();

	if (runtimeCfg.profiling) {
		HR_LOG(info) << "" << *rootProfiler;
		HR_LOG(info) << "  " << *advanceProfiler;
		HR_LOG(info) << "  " << *prepareProfiler;
		HR_LOG(info) << "  " << *renderProfiler;
	}

	return retv;
}

void ClientApp::OnConsoleToggle()
{
	if (!Config::GetInstance()->runtime.enableConsole) return;

	if (auto scene = consoleScene.lock()) {
		Scene::Phase phase = scene->GetPhase();
		if (phase != Scene::Phase::STOPPING &&
			phase != Scene::Phase::STOPPED)
		{
			return;
		}
	}

	auto newConsoleScene = std::make_shared<HoverScript::ConsoleScene>(*display, *this, *sysConsole);
	consoleScene = newConsoleScene;
	PushScene(newConsoleScene);
}

/**
 * Remove the foreground scene.
 * @note This must only be called from the main thread.
 * This indicates that there is no scene capable of receiving input.
 */
void ClientApp::SetForegroundScene()
{
	// Detach the controller from the previous foreground scene.
	if (fgScene) {
		consoleToggleConn.disconnect();
		fgScene->DetachInput(*controller);
	}

	// Don't display the overlay scene since we're in an unusual state and
	// probably can't properly deal with overlay interactions.
	if (showOverlay) {
		statusOverlayScene->DetachInput(*controller);
		statusOverlayScene->MoveToBackground();
		showOverlay = false;
	}

	fgScene.reset();

	controller->ClearActionMap();

	// Enable the console toggle.
	controller->AddConsoleToggleMaps();
	consoleToggleConn = controller->actions.sys.consoleToggle->Connect(
		std::bind(&ClientApp::OnConsoleToggle, this));

	// Enable the cursor to make it easier for users to click the "Close"
	// button on the main window.
	SDL_ShowCursor(SDL_ENABLE);
}

/**
 * Switch which scene has input focus.
 * @note This must only be called from the main thread.
 * @param scene The new foreground scene.
 */
void ClientApp::SetForegroundScene(const std::shared_ptr<Scene> &scene)
{
	if (!scene) {
		SetForegroundScene();
	}
	else {
		bool uiMode = scene->IsMouseCursorEnabled();

		// Detach the controller from the previous foreground scene.
		if (fgScene) {
			consoleToggleConn.disconnect();
			fgScene->DetachInput(*controller);
			fgScene->MoveToBackground();
		}
		if (showOverlay && !uiMode) {
			statusOverlayScene->DetachInput(*controller);
			statusOverlayScene->MoveToBackground();
		}

		fgScene = scene;

		// Load controller mapping from new foreground scene.
		controller->ClearActionMap();
		scene->MoveToForeground();
		scene->AttachInput(*controller);
		if (!showOverlay && uiMode) {
			statusOverlayScene->MoveToForeground();
			statusOverlayScene->AttachInput(*controller);
		}

		showOverlay = uiMode;

		// Enable the console toggle.
		controller->AddConsoleToggleMaps();
		consoleToggleConn = controller->actions.sys.consoleToggle->Connect(
			std::bind(&ClientApp::OnConsoleToggle, this));

		SDL_ShowCursor(uiMode ? SDL_ENABLE : SDL_DISABLE);
	}
}

/**
 * Push a new scene to the foreground.
 * @note This must only be called from the main thread.
 * @param scene The scene to push.
 */
void ClientApp::PushScene(const std::shared_ptr<Scene> &scene)
{
	// If the console is visible, then the new scene replaces it.
	if (Config::GetInstance()->runtime.enableConsole) {
		if (auto cscene = consoleScene.lock()) {
			if (cscene.get() != scene.get()) {
				Scene::Phase phase = cscene->GetPhase();
				if (phase != Scene::Phase::STOPPING &&
					phase != Scene::Phase::STOPPED)
				{
					PopScene();
				}
			}
		}
	}

	sceneStack.push_back(scene);
	scene->SetPhase(Scene::Phase::STARTING);
	SetForegroundScene(sceneStack.back());
	scene->OnScenePushed();
}

/**
 * Return to the previous scene, if any.
 * @note This must only be called from the main thread.
 */
void ClientApp::PopScene()
{
	if (fgScene) {
		fgScene->SetPhase(Scene::Phase::STOPPING);

		// Find the first non-stopping scene.
		bool found = false;
		for (auto iter = sceneStack.rbegin(); iter != sceneStack.rend(); ++iter) {
			Scene::Phase phase = (*iter)->GetPhase();
			if (phase != Scene::Phase::STOPPING && phase != Scene::Phase::STOPPED) {
				SetForegroundScene(*iter);
				found = true;
				break;
			}
		}

		if (!found) {
			HR_LOG(error) << "No foreground scene; entering failsafe mode.";
			SetForegroundScene();
		}
	}
}

/**
 * Replace the current scene and all background scenes with a new
 * foreground scene.
 * @note This must only be called from the main thread.
 * @param scene The scene to push.
 */
void ClientApp::ReplaceScene(const std::shared_ptr<Scene> &scene)
{
	for (auto &s : sceneStack) {
		s->SetPhase(Scene::Phase::STOPPING);
	}
	PushScene(scene);
}

/**
 * Immediately remove all scenes.
 * @note This must only be called from the main thread.
 */
void ClientApp::TerminateAllScenes()
{
	SetForegroundScene();
	sceneStack.clear();
}

void ClientApp::RequestPushScene(const std::shared_ptr<Scene> &scene)
{
	SDL_Event evt;
	evt.type = userEventId;
	evt.user.code = REQ_EVT_SCENE_PUSH;
	evt.user.data1 = new SceneHolder(scene);
	SDL_PushEvent(&evt);
}

void ClientApp::RequestPopScene()
{
	SDL_Event evt;
	evt.type = userEventId;
	evt.user.code = REQ_EVT_SCENE_POP;
	SDL_PushEvent(&evt);
}

void ClientApp::RequestReplaceScene(const std::shared_ptr<Scene> &scene)
{
	SDL_Event evt;
	evt.type = userEventId;
	evt.user.code = REQ_EVT_SCENE_REPLACE;
	evt.user.data1 = new SceneHolder(scene);
	SDL_PushEvent(&evt);
}

void ClientApp::RequestMainMenu(std::shared_ptr<LoadingScene> loadingScene)
{
	// Load the animated background scene, which is actually just a track without
	// the HUD or game sounds.
	// The menu scene is overlaid on top of the background scene.

	if (!loadingScene) {
		loadingScene = std::make_shared<LoadingScene>(*display, *this, "Load");
	}

	try {
		RequestReplaceScene(std::make_shared<DemoGameScene>(
			*display, *this, scripting.get(), loadingScene->ShareLoader()));
	}
	catch (Parcel::ObjStreamExn&) {
		throw;
	}

	RequestPushScene(std::make_shared<MainMenuScene>(*display, *this, *rulebookLibrary));

	loadingScene->ShareLoader()->GetFinishedLoadingSignal().connect([&]() {
		const auto cfg = Config::GetInstance();

		if (needsLocaleCheck && !cfg->GetLocale().GetSelectedLocaleId()) {
			needsLocaleCheck = false; // Only display warning once.

			// This message is intentionally not translatable --
			// we're here precisely because the locale isn't available.
			RequestPushScene(std::make_shared<MessageScene>(*display, *this,
				"Locale not available",
				"Your preferred locale (" +
				Locale::NormalizeId(cfg->i18n.preferredLocale) +
				") is not installed.\n"
				"\n"
				"You may select a new locale from the Settings menu."));
		}
		if (needsDevWarning) {
			needsDevWarning = false;
			RequestPushScene(std::make_shared<MessageScene>(*display, *this,
				"HoverRace 2.0 Developer Preview",
				"This is the unstable 2.0 branch of HoverRace.\n"
				"For the stable branch, switch to the \"1.24\" branch.\n"
				"\n"
				"To skip this warning, use --skip-startup-warning or specify a "
				"startup script with --exec."));
		}
	});

	RequestPushScene(loadingScene);
}

void ClientApp::RequestNewPracticeSession(std::shared_ptr<Rules> rules,
	std::shared_ptr<LoadingScene> loadingScene)
{
	if (!loadingScene) {
		loadingScene = std::make_shared<LoadingScene>(*display, *this, "Load");
	}

	try {
		RequestReplaceScene(std::make_shared<PlayGameScene>(
			*display, *this, scripting.get(), rules, loadingScene->ShareLoader()));
		RequestPushScene(loadingScene);
	}
	catch (Parcel::ObjStreamExn&) {
		throw;
	}
}

void ClientApp::RequestStatusPresentation()
{
	if (statusOverlayScene) {
		statusOverlayScene->PresentInitialState();
	}
}

void ClientApp::RequestAnnouncement(std::shared_ptr<Announcement> ann)
{
	SDL_Event evt;
	evt.type = userEventId;
	evt.user.code = REQ_EVT_ANN_ADD;
	evt.user.data1 = new AnnouncementHolder(ann);
	SDL_PushEvent(&evt);
}

void ClientApp::RequestShutdown()
{
	SDL_Event evt;
	evt.type = SDL_QUIT;
	SDL_PushEvent(&evt);
}

void ClientApp::RequestSoftRestart()
{
	SDL_Event evt;
	evt.type = userEventId;
	evt.user.code = REQ_EVT_SOFT_RESTART;
	SDL_PushEvent(&evt);
}

VideoServices::VideoBuffer *ClientApp::GetVideoBuffer() const
{
	return &display->GetLegacyDisplay();
}

Control::InputEventController *ClientApp::ReloadController()
{
	//TODO: Notify current scene that controller is changing.
	controller.reset(new InputEventController());
	return controller.get();
}

std::shared_ptr<Player::Player> ClientApp::ShareUiPilot() const
{
	//TODO: Keep track of which of the local players is in control.
	// For now, we assume all players are local and the first player in the
	// roster is the only human.
	return party->ShareFirst();
}

}  // namespace HoverScript
}  // namespace Client
