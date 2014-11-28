
// ClientApp.cpp
//
// Copyright (c) 2010, 2013, 2014 Michael Imamura.
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

#include "../StdAfx.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "../../engine/Exception.h"
#include "../../engine/Control/Controller.h"
#include "../../engine/Display/Label.h"
#include "../../engine/Display/UiLayoutFlags.h"
#include "../../engine/Display/SDL/SdlDisplay.h"
#include "../../engine/MainCharacter/MainCharacter.h"
#include "../../engine/Model/Track.h"
#include "../../engine/Parcel/TrackBundle.h"
#include "../../engine/Player/DemoProfile.h"
#include "../../engine/Player/LocalPlayer.h"
#include "../../engine/Util/Config.h"
#include "../../engine/Util/DllObjectFactory.h"
#include "../../engine/Util/FuzzyLogic.h"
#include "../../engine/Util/Loader.h"
#include "../../engine/Util/Str.h"
#include "../../engine/Util/WorldCoordinates.h"
#include "../../engine/VideoServices/SoundServer.h"
#include "../../engine/VideoServices/VideoBuffer.h"

#include "HoverScript/ClientScriptCore.h"
#include "HoverScript/ConsoleScene.h"
#include "HoverScript/DebugPeer.h"
#include "HoverScript/GamePeer.h"
#include "HoverScript/InputPeer.h"
#include "HoverScript/SessionPeer.h"
#include "HoverScript/SysConsole.h"
#include "HoverScript/SysEnv.h"
#include "ClientSession.h"
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
	};

	/**
	 * Used by @c REQ_EVT_SCENE_PUSH and @c REQ_SCENE_REPLACE to transfer
	 * the requested scene.
	 */
	struct SceneHolder
	{
		SceneHolder(const ScenePtr &scene) : scene(scene) { }
		ScenePtr scene;
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
}

ClientApp::ClientApp() :
	SUPER(),
	needsDevWarning(false),
	userEventId(SDL_RegisterEvents(1)),
	sceneStack(), fgScene(), statusOverlayScene(), showOverlay(false),
	fpsLbl(), frameCount(0), lastTimestamp(0), fps(0.0)
{
	Config *cfg = Config::GetInstance();

	// Engine initialization.
	MR_InitTrigoTables();
	MR_InitFuzzyModule();
	DllObjectFactory::Init();
	MainCharacter::MainCharacter::RegisterFactory();

	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) == -1)
		throw Exception("SDL initialization failed");
	SoundServer::Init();

	int imgInit = IMG_INIT_JPG | IMG_INIT_PNG;
	int imgInitActual = IMG_Init(imgInit);
	if ((imgInitActual & imgInit) != imgInit) {
		throw Exception(IMG_GetError());
	}

	if (TTF_Init() == -1) {
		throw Exception(TTF_GetError());
	}

	controller = new InputEventController();

	party = new Roster();

	// Add an initial single player with a fake profile.
	// Later, this will be done at the title scene, but for now we'll have
	// this hard-coded.
	party->AddPlayer(std::make_shared<Player::LocalPlayer>(
		std::make_shared<Player::DemoProfile>(), true, true));

	// Create the system console and execute the initialization scripts.
	// This allows the script to modify the configuration (e.g. for unit tests).
	scripting = (new ClientScriptCore())->Reset();
	rulebookLibrary = new RulebookLibrary(scripting);
	rulebookLibrary->Reload();
	debugPeer = new DebugPeer(scripting, *this);
	gamePeer = new GamePeer(scripting, *this, *rulebookLibrary);
	inputPeer = new InputPeer(scripting, controller);
	sysEnv = new SysEnv(scripting, debugPeer, gamePeer, inputPeer);
	for (OS::path_t &initScript : cfg->runtime.initScripts) {
		if (!initScript.empty()) {
			sysEnv->RunScript(initScript);
		}
	}
	sysConsole = new SysConsole(scripting, *this,
		debugPeer, gamePeer, inputPeer);

	//TODO: Select which display to use.
	SDL_DisplayMode desktopMode;
	if (SDL_GetDesktopDisplayMode(0, &desktopMode) < 0) {
		throw Exception(SDL_GetError());
	}

	// Create the main window and SDL surface.
	//TODO: Select which display to use.
	display = new Display::SDL::SdlDisplay(GetWindowTitle());
	display->OnDesktopModeChanged(desktopMode.w, desktopMode.h);

	// Set window position and icon (platform-dependent).
	// We don't throw an exception if this fails since it's not critical.
	//TODO: Move to SdlDisplay.
	SDL_SysWMinfo wm;
	SDL_VERSION(&wm.version);
	if (SDL_GetWindowWMInfo(static_cast<Display::SDL::SdlDisplay*>(display)->GetWindow(), &wm)) {
#		ifdef _WIN32
			HWND hwnd = wm.info.win.window;

			// Set icon.
			// On Windows, the icon is embedded as a resource.
			HMODULE hmod = GetModuleHandleW(NULL);
			LPWSTR iconRes = MAKEINTRESOURCEW(IDI_HOVER_ICON);
			HANDLE ico;
			ico = LoadImageW(hmod, iconRes, IMAGE_ICON,
				GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
			if (ico != NULL)
				SendMessageW(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(ico));
			ico = LoadImageW(hmod, iconRes, IMAGE_ICON,
				GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
			if (ico != NULL)
				SendMessageW(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(ico));
#		else
			// On non-Win32 we prefer to let the window manager decide the
			// position of the window.
			/*TODO
			SDL_WM_SetIcon(SDL_LoadBMP(cfg->GetMediaPath("icon.bmp").file_string().c_str()), 0);
			*/
#		endif
	}
	else {
		throw Exception(SDL_GetError());
	}

	gamePeer->SetDisplay(display);

	namespace LayoutFlags = Display::UiLayoutFlags;
	fpsLbl = new Display::Label("FPS:",
		Display::UiFont(20, Display::UiFont::BOLD),
		Display::Color(0xff, 0xff, 0x7f, 0x00),
		LayoutFlags::UNSCALED | LayoutFlags::FLOATING);
	fpsLbl->AttachView(*display);

	// SDL2 automatically initiates text input mode.
	// We want to have each scene decide whether they expect text input or not.
	SDL_StopTextInput();
}

ClientApp::~ClientApp()
{
	gamePeer->SetDisplay(nullptr);

	delete fpsLbl;
	delete sysConsole;
	delete sysEnv;
	delete inputPeer;
	delete gamePeer;
	delete debugPeer;
	delete rulebookLibrary;
	delete scripting;
	delete display;
	delete party;
	delete controller;

	// Engine shutdown.
	DllObjectFactory::Clean(FALSE);
	SoundServer::Close();

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

std::string ClientApp::GetWindowTitle()
{
	Config *cfg = Config::GetInstance();

	std::ostringstream oss;
	oss << PACKAGE_NAME " " << cfg->GetVersion();
	if (cfg->IsPrerelease()) {
		oss << " (" << pgettext("Version", "testing") << ')';
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
	auto iter = sceneStack.begin();
	while (iter != sceneStack.end()) {
		Scene &scene = **iter;
		Scene::Phase phase = scene.GetPhase();
		if (phase != Scene::Phase::INITIALIZING &&
			phase != Scene::Phase::STOPPED)
		{
			scene.Advance(tick);
		}

		// If the scene has entered the STOPPED state, it's deletable.
		if (scene.GetPhase() == Scene::Phase::STOPPED) {
			iter = sceneStack.erase(iter);
		}
		else {
			++iter;
		}
	}

	if (showOverlay) statusOverlayScene->Advance(tick);
}

void ClientApp::RenderScenes()
{
	bool showFps = Config::GetInstance()->runtime.showFramerate;

	IncFrameCount();

	if (sceneStack.empty()) {
		VideoServices::VideoBuffer::Lock lock(display->GetLegacyDisplay());
		display->GetLegacyDisplay().Clear();
	}
	else {
		for (const ScenePtr &scene : sceneStack) {
			scene->PrepareRender();
		}
		if (showOverlay) statusOverlayScene->PrepareRender();
		if (showFps) fpsLbl->PrepareRender();

		for (const ScenePtr &scene : sceneStack) {
			Scene::Phase phase = scene->GetPhase();
			if (phase != Scene::Phase::INITIALIZING &&
				phase != Scene::Phase::STOPPED)
			{
				scene->Render();
			}
		}
		if (showOverlay) statusOverlayScene->Render();
		if (showFps) fpsLbl->Render();
	}

	display->Flip();
}

void ClientApp::MainLoop()
{
	bool quit = false;
	SDL_Event evt;

	statusOverlayScene.reset(new StatusOverlayScene(*display, *this));

	const auto &runtimeCfg = Config::GetInstance()->runtime;
	needsDevWarning =
		!runtimeCfg.skipStartupWarning &&
		runtimeCfg.initScripts.empty();

	RequestMainMenu();

	// Fire all on_init handlers.
	gamePeer->OnInit();

	while (!quit) {
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
		RenderScenes();
	}

	TerminateAllScenes();
	statusOverlayScene.reset();

	gamePeer->OnShutdown();
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
		fgScene->DetachController(*controller);
	}

	// Don't display the overlay scene since we're in an unusual state and
	// probably can't properly deal with overlay interactions.
	if (showOverlay) {
		statusOverlayScene->DetachController(*controller);
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
void ClientApp::SetForegroundScene(const ScenePtr &scene)
{
	if (!scene) {
		SetForegroundScene();
	}
	else {
		bool uiMode = scene->IsMouseCursorEnabled();

		// Detach the controller from the previous foreground scene.
		if (fgScene) {
			consoleToggleConn.disconnect();
			fgScene->DetachController(*controller);
			fgScene->MoveToBackground();
		}
		if (showOverlay && !uiMode) {
			statusOverlayScene->DetachController(*controller);
			statusOverlayScene->MoveToBackground();
		}

		fgScene = scene;

		// Load controller mapping from new foreground scene.
		controller->ClearActionMap();
		scene->MoveToForeground();
		scene->AttachController(*controller);
		if (!showOverlay && uiMode) {
			statusOverlayScene->MoveToForeground();
			statusOverlayScene->AttachController(*controller);
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
void ClientApp::PushScene(const ScenePtr &scene)
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
			Log::Error("No foreground scene; entering failsafe mode.");
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
void ClientApp::ReplaceScene(const ScenePtr &scene)
{
	for (ScenePtr &s : sceneStack) {
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

void ClientApp::RequestPushScene(const ScenePtr &scene)
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

void ClientApp::RequestReplaceScene(const ScenePtr &scene)
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
			*display, *this, scripting, loadingScene->GetLoader()));
	}
	catch (Parcel::ObjStreamExn&) {
		throw;
	}

	RequestPushScene(std::make_shared<MainMenuScene>(*display, *this, *rulebookLibrary));

	loadingScene->GetLoader()->GetFinishedLoadingSignal().connect([&]() {
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
			*display, *this, scripting, rules, loadingScene->GetLoader()));
		RequestPushScene(loadingScene);
	}
	catch (Parcel::ObjStreamExn&) {
		throw;
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

VideoServices::VideoBuffer *ClientApp::GetVideoBuffer() const
{
	return &display->GetLegacyDisplay();
}

Control::InputEventController *ClientApp::ReloadController()
{
	//TODO: Notify current scene that controller is changing.
	delete controller;
	return (controller = new InputEventController());
}

}  // namespace HoverScript
}  // namespace Client
