
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
#include "../../engine/Display/Hud.h"
#include "../../engine/Model/Track.h"
#include "../../engine/Parcel/TrackBundle.h"
#include "../../engine/VideoServices/SoundServer.h"
#include "../../engine/VideoServices/VideoBuffer.h"

#include "HoverScript/GamePeer.h"
#include "HoverScript/HudPeer.h"
#include "HoverScript/MetaPlayer.h"
#include "HoverScript/MetaSession.h"
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

GameScene::Viewport::Viewport(Display::Display &display, Observer *observer,
                              Display::Hud *hud) :
	observer(observer), hud(hud)
{
	hud->AttachView(display);
}

GameScene::GameScene(Display::Display &display, GameDirector &director,
                     Script::Core *scripting, std::shared_ptr<Rules> rules) :
	SUPER("Game"),
	display(display), director(director), rules(rules),
	muted(false),
	session(nullptr),
	firedOnStart(false), firedOnRaceFinish(false)
{
	auto rulebook = rules->GetRulebook();

	// Create the new session
	session = new ClientSession(rules);

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
	metaSession = rulebook->GetMetas().session(
		std::make_shared<SessionPeer>(scripting, session));
	metaSession->OnInit();
	director.GetSessionChangedSignal()(metaSession);

	//TODO: Support split-screen with multiple viewports.
	viewports.emplace_back(
		display,
		new Observer(),
		new Display::Hud(display, session->GetPlayer(0),
			Vec2(1280, 720)));

	metaSession->OnPregame();
	auto sessionPeer = metaSession->GetSession();
	sessionPeer->ForEachPlayer([&](std::shared_ptr<MetaPlayer> &player) {
		auto playerPeer = player->GetPlayer();
		//TODO: Look up the correct HUD for this player.
		playerPeer->SetHud(std::make_shared<HudPeer>(scripting, display,
			viewports.back().hud));
		player->OnJoined(metaSession);
	});
}

GameScene::~GameScene()
{
	Cleanup();
}

void GameScene::Cleanup()
{
	director.GetSessionChangedSignal()(nullptr);
	delete session;
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
	BOOST_FOREACH(auto &viewport, viewports) {
		viewport.observer->Zoom(increment);
	}
}

void GameScene::OnCameraPan(int increment)
{
	BOOST_FOREACH(auto &viewport, viewports) {
		viewport.observer->Scroll(increment);
	}
}

void GameScene::OnCameraReset()
{
	BOOST_FOREACH(auto &viewport, viewports) {
		viewport.observer->Home();
	}
}

/// Fired when the pause button (e.g. ESC) is pressed.
void GameScene::OnPause()
{
	director.RequestPushScene(std::make_shared<PauseMenuScene>(display, director));
}

void GameScene::SetHudVisible(bool visible)
{
	BOOST_FOREACH(auto &viewport, viewports) {
		viewport.observer->SetHudVisible(visible);
		viewport.hud->SetVisible(visible);
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

	BOOST_FOREACH(auto &viewport, viewports) {
		viewport.observer->StartDemoMode();
	}
}

void GameScene::Advance(Util::OS::timestamp_t tick)
{
	SUPER::Advance(tick);

	session->Process();

	if (!firedOnRaceFinish && session->GetPlayer(0)->HasFinish()) {
		metaSession->GetSession()->GetPlayer(0)->OnFinish();
		OnRaceFinish();
		firedOnRaceFinish = true;
	}
	else if (!firedOnStart && session->GetPlayer(0)->HasStarted()) {
		metaSession->GetSession()->GetPlayer(0)->OnStart();
		firedOnStart = true;
	}

	// Update HUD state last, after game state is settled for this frame.
	BOOST_FOREACH(auto &viewport, viewports) {
		viewport.hud->Advance(tick);
	}
}

void GameScene::PrepareRender()
{
	SUPER::PrepareRender();

	BOOST_FOREACH(auto &viewport, viewports) {
		viewport.hud->PrepareRender();
	}
}

void GameScene::Render()
{
	MR_SimulationTime simTime = session->GetSimulationTime();

	{
		VideoServices::VideoBuffer *videoBuf = &display.GetLegacyDisplay();
		VideoServices::VideoBuffer::Lock lock(*videoBuf);

		int i = 0;
		BOOST_FOREACH(auto &viewport, viewports) {
			viewport.observer->RenderNormalDisplay(videoBuf, session,
				session->GetPlayer(i++),
				simTime, session->GetBackImage());
		}
	}
	BOOST_FOREACH(auto &viewport, viewports) {
		if (viewport.hud->IsVisible()) {
			viewport.hud->Render();
		}
	}

	// Trigger sounds.
	if (!muted) {
		int i = 0;
		BOOST_FOREACH(auto &viewport, viewports) {
			viewport.observer->PlaySounds(session->GetCurrentLevel(),
				session->GetPlayer(i++));
		}
		VideoServices::SoundServer::ApplyContinuousPlay();
	}
}

void GameScene::OnRaceFinish()
{
	metaSession->OnPostgame();
	director.GetSessionChangedSignal()(nullptr);
}

}  // namespace HoverScript
}  // namespace Client
