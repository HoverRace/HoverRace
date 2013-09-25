
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

#include "../../engine/Control/Controller.h"
#include "../../engine/Display/Display.h"
#include "../../engine/Model/Track.h"
#include "../../engine/Parcel/TrackBundle.h"
#include "../../engine/VideoServices/SoundServer.h"
#include "../../engine/VideoServices/VideoBuffer.h"

#include "HoverScript/GamePeer.h"
#include "HoverScript/PlayerPeer.h"
#include "HoverScript/SessionPeer.h"

#include "ClientSession.h"
#include "PauseMenuScene.h"
#include "Rulebook.h"
#include "Rules.h"

#include "GameScene.h"

using namespace HoverRace::Client::HoverScript;
using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

GameScene::GameScene(Display::Display &display, GameDirector &director,
                     Script::Core *scripting, HoverScript::GamePeer *gamePeer,
                     std::shared_ptr<Rules> rules) :
	SUPER("Game"),
	display(display), director(director), gamePeer(gamePeer), rules(rules),
	frame(0), numPlayers(1), muted(false),
	session(nullptr),
	firedOnRaceFinish(false)
{
	memset(observers, 0, sizeof(observers[0]) * MAX_OBSERVERS);

	// Create the new session
	session = new ClientSession(rules);
	sessionPeer = std::make_shared<SessionPeer>(scripting, session);

	// Load the selected track
	try {
		auto entry = rules->GetTrackEntry();
		auto track = Config::GetInstance()->GetTrackBundle()->OpenTrack(entry);
		if (!track) throw Parcel::ObjStreamExn("Track does not exist.");
		if (!session->LoadNew(
			entry->name.c_str(), track->GetRecordFile(),
			&display.GetLegacyDisplay()))
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

	gamePeer->OnSessionStart(sessionPeer);
	rules->GetRulebook()->OnPreGame(sessionPeer);
}

GameScene::~GameScene()
{
	Cleanup();
}

void GameScene::Cleanup()
{
	delete session;
	for (int i = 0; i < numPlayers; i++) {
		if (observers[i] != NULL) {
			observers[i]->Delete();
		}
	}
}

void GameScene::AttachController(Control::InputEventController &controller)
{
	MainCharacter::MainCharacter* mc = session->GetPlayer(0);
	controller.AddPlayerMaps(1, &mc);
	controller.AddCameraMaps();

	//TODO: Use separate action for pausing than ui.menuCancel.
	controller.AddMenuMaps();

	cameraZoomInConn = controller.actions.camera.zoomIn->Connect(
		std::bind(&GameScene::OnCameraZoom, this, 1));
	cameraZoomOutConn = controller.actions.camera.zoomOut->Connect(
		std::bind(&GameScene::OnCameraZoom, this, -1));
	cameraPanUpConn = controller.actions.camera.panUp->Connect(
		std::bind(&GameScene::OnCameraPan, this, 1));
	cameraPanDownConn = controller.actions.camera.panDown->Connect(
		std::bind(&GameScene::OnCameraPan, this, -1));
	cameraResetConn = controller.actions.camera.reset->Connect(
		std::bind(&GameScene::OnCameraReset, this));

	pauseConn = controller.actions.ui.menuCancel->Connect(
		std::bind(&GameScene::OnPause, this));
}

void GameScene::DetachController(Control::InputEventController &controller)
{
	// Shut off the engine when the controller is detached (e.g. when showing a
	// dialog) otherwise we'll just keep accelerating into the wall.
	MainCharacter::MainCharacter* mc = session->GetPlayer(0);
	mc->SetEngineState(false);
	mc->SetBrakeState(false);
	mc->SetTurnLeftState(false);
	mc->SetTurnRightState(false);
	mc->SetLookBackState(false);

	pauseConn.disconnect();

	cameraResetConn.disconnect();
	cameraPanDownConn.disconnect();
	cameraPanUpConn.disconnect();
	cameraZoomOutConn.disconnect();
	cameraZoomInConn.disconnect();
}

void GameScene::OnCameraZoom(int increment)
{
	for (int i = 0; i < MAX_OBSERVERS; ++i) {
		Observer *obs = observers[i];
		if (obs != NULL) {
			obs->Zoom(increment);
		}
	}
}

void GameScene::OnCameraPan(int increment)
{
	for (int i = 0; i < MAX_OBSERVERS; ++i) {
		Observer *obs = observers[i];
		if (obs != NULL) {
			obs->Scroll(increment);
		}
	}
}

void GameScene::OnCameraReset()
{
	for (int i = 0; i < MAX_OBSERVERS; ++i) {
		Observer *obs = observers[i];
		if (obs != NULL) {
			obs->Home();
		}
	}
}

/// Fired when the pause button (e.g. ESC) is pressed.
void GameScene::OnPause()
{
	director.RequestPushScene(std::make_shared<PauseMenuScene>(display, director));
}

void GameScene::SetHudVisible(bool visible)
{
	for (int i = 0; i < MAX_OBSERVERS; ++i) {
		Observer *obs = observers[i];
		if (obs) {
			obs->SetHudVisible(visible);
		}
	}
}

void GameScene::SetMuted(bool muted)
{
	this->muted = muted;
}

void GameScene::StartDemoMode()
{
	SetHudVisible(false);
	SetMuted(true);

	for (int i = 0; i < MAX_OBSERVERS; ++i) {
		Observer *obs = observers[i];
		if (obs) {
			obs->StartDemoMode();
		}
	}
}

void GameScene::Advance(Util::OS::timestamp_t tick)
{
	SUPER::Advance(tick);

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

	// Trigger sounds.
	if (!muted) {
		for (int i = 0; i < MAX_OBSERVERS; ++i) {
			Observer *obs = observers[i];
			if (obs != NULL) {
				obs->PlaySounds(session->GetCurrentLevel(), session->GetPlayer(i));
			}
		}
		VideoServices::SoundServer::ApplyContinuousPlay();
	}
}

void GameScene::OnRaceFinish()
{
	rules->GetRulebook()->OnPostGame(sessionPeer);
	gamePeer->OnSessionEnd(sessionPeer);
}

}  // namespace HoverScript
}  // namespace Client
