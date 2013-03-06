
// GameScene.cpp
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

#include "StdAfx.h"

#include "../../engine/Display/Display.h"
#include "../../engine/Model/Track.h"
#include "../../engine/Parcel/TrackBundle.h"
#include "../../engine/VideoServices/SoundServer.h"
#include "../../engine/VideoServices/VideoBuffer.h"

#include "Control/Controller.h"
#include "HoverScript/GamePeer.h"
#include "HoverScript/SessionPeer.h"

#include "ClientSession.h"
#include "HighObserver.h"
#include "MessageScene.h"
#include "Rulebook.h"

#include "GameScene.h"

using namespace HoverRace::Client::HoverScript;
using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

GameScene::GameScene(GameDirector *director, Display::Display &display,
                     Script::Core *scripting, HoverScript::GamePeer *gamePeer,
                     RulebookPtr rules) :
	SUPER("Game"),
	director(director),
	frame(0), numPlayers(1), display(display),
	session(NULL), highObserver(NULL), highConsole(NULL),
	firedOnRaceFinish(false)
{
	memset(observers, 0, sizeof(observers[0]) * MAX_OBSERVERS);

	// Create the new session
	session = new ClientSession();
	sessionPeer = std::make_shared<SessionPeer>(scripting, session);

	// Load the selected track
	try {
		Model::TrackPtr track = Config::GetInstance()->
			GetTrackBundle()->OpenTrack(rules->GetTrackName());
		if (track.get() == NULL) throw Parcel::ObjStreamExn("Track does not exist.");
		if (!session->LoadNew(
			rules->GetTrackName().c_str(), track->GetRecordFile(),
			rules->GetLaps(), rules->GetGameOpts(), &display.GetLegacyDisplay()))
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

void GameScene::SetupController(Control::InputEventController &controller)
{
	MainCharacter::MainCharacter* mc = session->GetPlayer(0);
	controller.AddPlayerMaps(1, &mc);
	controller.AddObserverMaps(observers, 1);

	if (highConsole) {
		controller.SetConsole(highConsole);
	}
}

void GameScene::Advance(Util::OS::timestamp_t tick)
{
	if (highConsole != NULL && highConsole->IsVisible()) {
		highConsole->Advance(tick);
	}

	session->Process();

	if (!firedOnRaceFinish && session->GetPlayer(0)->HasFinish()) {
		OnRaceFinish();
		firedOnRaceFinish = true;
	}
}

void GameScene::Render()
{
	MR_SimulationTime simTime = session->GetSimulationTime();

	VideoServices::VideoBuffer *videoBuf = &display.GetLegacyDisplay();
	VideoServices::VideoBuffer::Lock lock(*videoBuf);

	for (int i = 0; i < MAX_OBSERVERS; ++i) {
		Observer *obs = observers[i];
		if (obs != NULL) {
			obs->RenderNormalDisplay(videoBuf, session,
				session->GetPlayer(i),
				simTime, session->GetBackImage());
		}
	}

	if (highObserver != NULL) {
		highObserver->Render(videoBuf, session);
	}
	if (highConsole != NULL) {
		highConsole->Render(videoBuf);
	}

	// Trigger sounds.
	for (int i = 0; i < MAX_OBSERVERS; ++i) {
		Observer *obs = observers[i];
		if (obs != NULL) {
			obs->PlaySounds(session->GetCurrentLevel(), session->GetPlayer(i));
		}
	}
	VideoServices::SoundServer::ApplyContinuousPlay();
}

void GameScene::OnRaceFinish()
{
	// This is just a test of scene manipulation.
	director->RequestPushScene(std::make_shared<MessageScene>(display,
		"Congratulations",
		"Race complete!"));
}

}  // namespace HoverScript
}  // namespace Client
