
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
#include "HoverScript/SessionPeer.h"

#include "ClientSession.h"
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
	session(nullptr),
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
}

void GameScene::DetachController(Control::InputEventController &controller)
{
	// Shut off the engine when the controller is detached (e.g. when showing a
	// dialog) otherwise we'll just keep accelerating into the wall.
	MainCharacter::MainCharacter* mc = session->GetPlayer(0);
	mc->SetEngineState(false);

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

void GameScene::OnPhaseChanged(Phase::phase_t oldPhase)
{
	// Act like the starting and stopping phases don't even exist.
	switch (GetPhase()) {
		case Phase::STARTING:
			SetPhase(Phase::RUNNING);
			break;
		case Phase::STOPPING:
			SetPhase(Phase::STOPPED);
			break;
	}
}

void GameScene::Advance(Util::OS::timestamp_t tick)
{
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
	director->RequestPushScene(std::make_shared<MessageScene>(display, *director,
		"Congratulations",
		"Race complete!"));
}

}  // namespace HoverScript
}  // namespace Client
