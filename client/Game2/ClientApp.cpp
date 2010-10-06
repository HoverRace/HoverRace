
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
using HoverRace::Client::Control::Controller;

namespace HoverRace {
namespace Client {

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
	currentSession(NULL)
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

	controller = new Controller(mainWnd, uiInput);

	RefreshTitleBar();
}

ClientApp::~ClientApp()
{
	delete currentSession;

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
	if (currentSession != NULL) {
		delete currentSession;
		currentSession = NULL;
	}

	//TODO: Prompt the user for a track name.

	observers[0] = Observer::New();
	highObserver = new HighObserver();

	// Create the new session
	ClientSession *newSession = new ClientSession();
	sessionPeer = boost::make_shared<SessionPeer>(scripting, newSession);

	if (Config::GetInstance()->runtime.enableConsole) {
		highConsole = new HighConsole(scripting, this, gamePeer, sessionPeer);
	}

	// Load the selected track
	try {
		Model::TrackPtr track = Config::GetInstance()->
			GetTrackBundle()->OpenTrack(rules->GetTrackName());
		if (track.get() == NULL) throw Parcel::ObjStreamExn("Track does not exist.");
		if (!newSession->LoadNew(
			rules->GetTrackName().c_str(), track->GetRecordFile(),
			rules->GetLaps(), rules->GetGameOpts(), videoBuf))
		{
			throw Parcel::ObjStreamExn("Track load failed.");
		}
	}
	catch (Parcel::ObjStreamExn&) {
		/*TODO
		TrackOpenFailMessageBox(mMainWindow, rules->GetTrackName(), ex.what());
		*/
		throw;
	}

	newSession->SetSimulationTime(-6000);

	if (!newSession->CreateMainCharacter(0)) {
		//TODO: Display error.
		return;
	}

	currentSession = newSession;

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

Control::Controller *ClientApp::ReloadController()
{
	delete controller;
	return (controller = new Controller(mainWnd, uiInput));
}

}  // namespace HoverScript
}  // namespace Client

#endif  // ifdef WITH_SDL
