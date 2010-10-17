
// ClientApp.cpp
// Experimental game shell.
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

#include "StdAfx.h"

#include <SDL/SDL_syswm.h>

#include "../../engine/Exception.h"
#include "../../engine/MainCharacter/MainCharacter.h"
#include "../../engine/Model/Track.h"
#include "../../engine/Parcel/TrackBundle.h"
#include "../../engine/Util/Config.h"
#include "../../engine/Util/FuzzyLogic.h"
#include "../../engine/Util/DllObjectFactory.h"
#include "../../engine/Util/Str.h"
#include "../../engine/Util/WorldCoordinates.h"
#include "../../engine/VideoServices/SoundServer.h"
#include "../../engine/VideoServices/VideoBuffer.h"

#include "Control/Controller.h"
#include "Control/UiHandler.h"
#include "HoverScript/ClientScriptCore.h"
#include "HoverScript/GamePeer.h"
#include "HoverScript/HighConsole.h"
#include "HoverScript/SessionPeer.h"
#include "HoverScript/SysEnv.h"
#include "ClientSession.h"
#include "HighObserver.h"
#include "Rulebook.h"

#ifdef _WIN32
#	include "resource.h"
#endif

#include "ClientApp.h"

#ifdef WITH_SDL

#ifdef WITH_OPENAL
#	define SOUNDSERVER_INIT(s) SoundServer::Init()
#else
#	define SOUNDSERVER_INIT(s) SoundServer::Init(s)
#endif

using namespace HoverRace::Client::HoverScript;
using namespace HoverRace::Util;
namespace SoundServer = HoverRace::VideoServices::SoundServer;
using HoverRace::Client::Control::InputEventController;

namespace HoverRace {
namespace Client {

class Scene
{
	public:
		virtual ~Scene() { }

	public:
		virtual void Advance(Util::OS::timestamp_t tick) = 0;
};

class GameScene : public Scene
{
	typedef Scene SUPER;
	public:
		GameScene(GameDirector *director, VideoServices::VideoBuffer *videoBuf,
			Script::Core *scripting, HoverScript::GamePeer *gamePeer,
			RulebookPtr rules);
		virtual ~GameScene();

	private:
		void Cleanup();

	public:
		void Advance(Util::OS::timestamp_t tick);

	private:
		int frame;
		int numPlayers;
		static const int MAX_OBSERVERS = Util::Config::MAX_PLAYERS;
		Observer *observers[MAX_OBSERVERS];
		ClientSession *session;

		HighObserver *highObserver;
		HoverScript::HighConsole *highConsole;

		HoverScript::SessionPeerPtr sessionPeer;
};

class ClientApp::UiInput : public Control::UiHandler
{
	virtual void OnConsole()
	{
		throw UnimplementedExn("ClientApp::UiInput::OnConsole()");
	}
};

ClientApp::ClientApp() :
	SUPER(),
	uiInput(boost::make_shared<UiInput>()),
	scene(NULL)
{
	Config *cfg = Config::GetInstance();

	// Engine initialization.
	MR_InitTrigoTables();
	MR_InitFuzzyModule();
	SOUNDSERVER_INIT(NULL);
	DllObjectFactory::Init();
	MainCharacter::MainCharacter::RegisterFactory();

	if (SDL_Init(SDL_INIT_VIDEO) == -1)
		throw Exception("SDL initialization failed");

	// Create the system console and execute the init script.
	// This allows the script to modify the configuration (e.g. for unit tests).
	scripting = (new ClientScriptCore())->Reset();
	gamePeer = new GamePeer(scripting, this);
	sysEnv = new SysEnv(scripting, gamePeer);
	OS::path_t &initScript = cfg->runtime.initScript;
	if (!initScript.empty()) {
		sysEnv->RunScript(initScript);
	}

	// With SDL we can only get the desktop resolution before the first call to
	// SDL_SetVideoMode().
	const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
	int desktopWidth = videoInfo->current_w;
	int desktopHeight = videoInfo->current_h;

	// Create the main window and SDL surface.
	if (SDL_SetVideoMode(cfg->video.xRes, cfg->video.yRes, 8,
		SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_RESIZABLE) == NULL)
	{
		throw Exception("Unable to create video surface");
	}

	// Set window position and icon (platform-dependent).
	SDL_SysWMinfo wm;
	SDL_VERSION(&wm.version);
	if (SDL_GetWMInfo(&wm) != 0) {
#		ifdef _WIN32
			HWND hwnd = mainWnd = wm.window;
			SetWindowPos(hwnd, HWND_TOP,
				cfg->video.xPos, cfg->video.yPos, 0, 0,
				SWP_NOSIZE);

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
			mainWnd = wm.info.x11.window;
			// On non-Win32 we prefer to let the window manager decide the
			// position of the window.
			/*TODO
			SDL_WM_SetIcon(SDL_LoadBMP(cfg->GetMediaPath("icon.bmp").file_string().c_str()), 0);
			*/
#		endif
	}

	videoBuf = new VideoServices::VideoBuffer(mainWnd,
		cfg->video.gamma, cfg->video.contrast, cfg->video.brightness);
	videoBuf->NotifyDesktopModeChange(desktopWidth, desktopHeight);

	AssignPalette();

	controller = new InputEventController(mainWnd, uiInput);

	RefreshTitleBar();
}

ClientApp::~ClientApp()
{
	delete scene;

	delete sysEnv;
	delete gamePeer;
	delete scripting;
	delete videoBuf;
	delete controller;

	// Engine shutdown.
	DllObjectFactory::Clean(FALSE);
	SoundServer::Close();

	SDL_Quit();
}

void ClientApp::RefreshTitleBar()
{
	Config *cfg = Config::GetInstance();

	std::ostringstream oss;
	oss << PACKAGE_NAME " " << cfg->GetVersion();
	if (cfg->IsPrerelease()) {
		oss << " (" << pgettext("Version", "testing") << ')';
	}
	if (cfg->runtime.silent) {
		oss << " (" << _("silent mode") << ')';
	}

	SDL_WM_SetCaption(oss.str().c_str(), NULL);
}

/**
 * Draws a color grid to debug the color palette.
 */
void ClientApp::DrawPalette()
{
	SDL_Surface *surface = SDL_GetVideoSurface();

	if (SDL_MUSTLOCK(surface)) {
		if (SDL_LockSurface(surface) < 0) {
			throw Exception("Unable to lock surface");
		}
	}

	MR_UInt8 *buf = static_cast<MR_UInt8*>(surface->pixels);
	for (int y = 0; y < 256; y++, buf += surface->pitch) {
		if ((y % 16) == 0) continue;
		MR_UInt8 *cur = buf;
		for (int x = 0; x < 256; x++, cur++) {
			if ((x % 16) == 0) continue;
			*cur = ((y >> 4) << 4) + (x >> 4);
		}
	}

	if (SDL_MUSTLOCK(surface)) {
		SDL_UnlockSurface(surface);
	}

	SDL_Flip(surface);
}

void ClientApp::MainLoop()
{
	bool quit = false;
	SDL_Event evt;

	// Fire all on_init handlers and check if a new session was requested.
	gamePeer->OnInit();
	RulebookPtr rules = gamePeer->RequestedNewSession();
	if (rules != NULL) {
		NewLocalSession(rules);
	}

	DrawPalette();

	while (!quit) {
		while (SDL_PollEvent(&evt)) {
			if (evt.type == SDL_QUIT) {
				quit = true;
			}
		}
	}
}

void ClientApp::NewLocalSession(RulebookPtr rules)
{
	//TODO: Confirm ending the current session.

	// Shut down the current session (if any).
	if (scene != NULL) {
		delete scene;
		scene = NULL;
	}

	//TODO: Prompt the user for a track name.
	try {
		scene = new GameScene(this, videoBuf, scripting, gamePeer, rules);
	}
	catch (Parcel::ObjStreamExn&) {
		throw;
	}

	AssignPalette();
}

void ClientApp::RequestShutdown()
{
	throw UnimplementedExn("ClientApp::RequestShutdown()");
}

void ClientApp::SignalServerHasChanged()
{
	throw UnimplementedExn("ClientApp::SignalServerHasChanged()");
}

void ClientApp::ChangeAutoUpdates(bool newSetting)
{
	throw UnimplementedExn("ClientApp::ChangeAutoUpdates(bool)");
}

void ClientApp::AssignPalette()
{
	Config *cfg = Config::GetInstance();

	videoBuf->CreatePalette(
		cfg->video.gamma, cfg->video.contrast, cfg->video.brightness);
	videoBuf->AssignPalette();
}

Control::InputEventController *ClientApp::ReloadController()
{
	delete controller;
	return (controller = new InputEventController(mainWnd, uiInput));
}

// GameScene

GameScene::GameScene(GameDirector *director, VideoServices::VideoBuffer *videoBuf,
                     Script::Core *scripting, HoverScript::GamePeer *gamePeer,
                     RulebookPtr rules) :
	SUPER(),
	frame(0), numPlayers(1), session(NULL), highObserver(NULL), highConsole(NULL)
{
	memset(observers, 0, sizeof(observers[0]) * MAX_OBSERVERS);

	// Create the new session
	session = new ClientSession();
	sessionPeer = boost::make_shared<SessionPeer>(scripting, session);

	// Load the selected track
	try {
		Model::TrackPtr track = Config::GetInstance()->
			GetTrackBundle()->OpenTrack(rules->GetTrackName());
		if (track.get() == NULL) throw Parcel::ObjStreamExn("Track does not exist.");
		if (!session->LoadNew(
			rules->GetTrackName().c_str(), track->GetRecordFile(),
			rules->GetLaps(), rules->GetGameOpts(), videoBuf))
		{
			throw Parcel::ObjStreamExn("Track load failed.");
		}
	}
	catch (Parcel::ObjStreamExn&) {
		Cleanup();
		throw;
	}

	session->SetSimulationTime(-6000);

	if (!session->CreateMainCharacter(0)) {
		Cleanup();
		throw Exception("Main character creation failed");
	}

	observers[0] = Observer::New();
	highObserver = new HighObserver();

	if (Config::GetInstance()->runtime.enableConsole) {
		highConsole = new HighConsole(scripting, director, gamePeer, sessionPeer);
	}

}

GameScene::~GameScene()
{
	Cleanup();
}

void GameScene::Cleanup()
{
	delete highConsole;
	delete highObserver;
	delete session;
	for (int i = 0; i < numPlayers; i++) {
		if (observers[i] != NULL) {
			observers[i]->Delete();
		}
	}
}

void GameScene::Advance(Util::OS::timestamp_t tick)
{
}

}  // namespace HoverScript
}  // namespace Client

#endif  // ifdef WITH_SDL
