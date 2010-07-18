
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
#include "../../engine/Util/Config.h"
#include "../../engine/Util/FuzzyLogic.h"
#include "../../engine/Util/DllObjectFactory.h"
#include "../../engine/Util/Str.h"
#include "../../engine/Util/WorldCoordinates.h"
#include "../../engine/VideoServices/SoundServer.h"

#include "Control/Controller.h"
#include "Control/UiHandler.h"
#include "HoverScript/ClientScriptCore.h"
#include "HoverScript/GamePeer.h"
#include "HoverScript/HighConsole.h"
#include "HoverScript/SessionPeer.h"
#include "HoverScript/SysEnv.h"

#include "ClientApp.h"

#ifdef WITH_OPENAL
#	define SOUNDSERVER_INIT(s) SoundServer::Init()
#else
#	define SOUNDSERVER_INIT(s) SoundServer::Init(s)
#endif

using namespace HoverRace::Client::HoverScript;
using namespace HoverRace::Util;
namespace SoundServer = HoverRace::VideoServices::SoundServer;

namespace HoverRace {
namespace Client {

ClientApp::ClientApp() :
	SUPER()
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

	// Create the main window and SDL surface.
	if (SDL_SetVideoMode(cfg->video.xRes, cfg->video.yRes, 8,
		SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_RESIZABLE) == NULL)
	{
		throw Exception("Unable to create video surface");
	}

	// Move the window to the saved position (platform-dependent).
	SDL_SysWMinfo wm;
	SDL_VERSION(&wm.version);
	if (SDL_GetWMInfo(&wm) != 0) {
#		ifdef _WIN32
			HWND hwnd = wm.window;
			SetWindowPos(hwnd, HWND_TOP,
				cfg->video.xPos, cfg->video.yPos, 0, 0,
				SWP_NOSIZE);
#		endif
	}

	RefreshTitleBar();
}

ClientApp::~ClientApp()
{
	delete sysEnv;
	delete gamePeer;
	delete scripting;

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

void ClientApp::MainLoop()
{
	bool quit = false;
	SDL_Event evt;

	while (!quit) {
		while (SDL_PollEvent(&evt)) {
			if (evt.type == SDL_QUIT) {
				quit = true;
			}
		}
	}
}

void ClientApp::RequestShutdown()
{
	//TODO
}

}  // namespace HoverScript
}  // namespace Client
