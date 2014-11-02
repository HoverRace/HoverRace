
// GameScene.cpp
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

#include "../../engine/Display/Display.h"
#include "../../engine/Display/Hud.h"
#include "../../engine/Model/Track.h"
#include "../../engine/Parcel/TrackBundle.h"
#include "../../engine/Player/Player.h"
#include "../../engine/Util/Duration.h"
#include "../../engine/Util/Loader.h"
#include "../../engine/VideoServices/SoundServer.h"
#include "../../engine/VideoServices/VideoBuffer.h"

#include "HoverScript/GamePeer.h"
#include "HoverScript/HudPeer.h"
#include "HoverScript/MetaPlayer.h"
#include "HoverScript/MetaSession.h"
#include "HoverScript/PlayerPeer.h"
#include "HoverScript/SessionPeer.h"

#include "ClientSession.h"
#include "Roster.h"
#include "Rulebook.h"
#include "Rules.h"

#include "GameScene.h"

using namespace HoverRace::Client::HoverScript;
using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

GameScene::Viewport::Viewport(Display::Display &display,
	std::shared_ptr<Player::Player> player, Observer *observer,
	Display::Hud *hud) :
	player(std::move(player)), observer(observer), hud(hud)
{
	hud->AttachView(display);
}

void GameScene::Viewport::SetCell(Display::HudCell cell)
{
	hud->SetCell(cell);
	observer->SetSplitMode(cell);
}

GameScene::GameScene(const std::string &name,
                     Display::Display &display, GameDirector &director,
                     Script::Core *scripting, std::shared_ptr<Rules> rules,
                     std::shared_ptr<Loader> loader) :
	SUPER(name),
	display(display), director(director), scripting(scripting), rules(rules),
	finishedLoading(false), muted(false),
	session(nullptr)
{
	finishedLoadingConn =
		loader->GetFinishedLoadingSignal().connect(
			std::bind(&GameScene::OnFinishedLoading, this));

	session = new ClientSession(rules);

	// Schedule the remaining load items.
	ScheduleLoad(loader);
}

GameScene::~GameScene()
{
	Cleanup();
}

void GameScene::Cleanup()
{
	director.GetSessionChangedSignal()(nullptr);
	if (metaSession) {
		metaSession->GetSession()->OnSessionEnd();
	}
	delete session;

	// Apply sound state to halt currently-playing continuous sounds.
	VideoServices::SoundServer::ApplyContinuousPlay();
}

void GameScene::ScheduleLoad(std::shared_ptr<Loader> loader)
{
	auto rulebook = rules->GetRulebook();

	loader->AddLoader("Track and players", [=]{
		// Load the selected track
		std::shared_ptr<Model::Track> track;

		auto entry = this->rules->GetTrackEntry();
		if (!entry) throw Parcel::ObjStreamExn("Track does not exist.");
		track = Config::GetInstance()->GetTrackBundle()->OpenTrack(entry);
		if (!track) throw Parcel::ObjStreamExn(
			"Track does not exist: " + entry->name);
		if (!session->LoadNew(entry->name.c_str(), scripting,
			track, &display.GetLegacyDisplay()))
		{
			throw Parcel::ObjStreamExn("Track load failed.");
		}

		// This must be done after the track has loaded.
		int i = 0;
		std::vector<std::shared_ptr<Player::Player>> localHumans;
		director.GetParty()->ForEach([&](std::shared_ptr<Player::Player> &p) {
			if (p->IsCompeting()) {
				session->AttachPlayer(i, p);
				i++;

				if (p->IsLocal() && p->IsHuman()) {
					localHumans.emplace_back(p);
				}
			}
		});

		// Split-screen with multiple viewports.
		// The bounds of each viewport will be set in LayoutViewports().
		for (auto &player : localHumans) {
			viewports.emplace_back(
				display,
				player,
				new Observer(),
				new Display::Hud(display,
					player, track,
					Display::UiLayoutFlags::FLOATING));
		}
	});

	loader->AddLoader("Session", [=]{
		metaSession = rulebook->GetMetas().session(
			std::make_shared<SessionPeer>(scripting, session));
		metaSession->OnInit();
		session->SetMeta(metaSession);

		director.GetSessionChangedSignal()(metaSession);

		session->AdvancePhase(ClientSession::Phase::PREGAME);

		auto sessionPeer = metaSession->GetSession();
		sessionPeer->ForEachPlayer([&](std::shared_ptr<MetaPlayer> &player) {
			auto playerPeer = player->GetPlayer();

			// Look up the correct HUD for this player.
			for (auto &viewport : viewports) {
				if (viewport.player.get() == playerPeer->GetPlayer()) {
					playerPeer->SetHud(
						std::make_shared<HudPeer>(scripting, display,
							viewport.hud));
					break;
				}
			}

			player->OnJoined(metaSession);
		});
	});
}

void GameScene::OnFinishedLoading()
{
	finishedLoading = true;

	RequestLayout();
}

void GameScene::SetHudVisible(bool visible)
{
	for (auto &viewport : viewports) {
		viewport.observer->SetHudVisible(visible);
		viewport.hud->SetVisible(visible);
	}
}

void GameScene::SetMuted(bool muted)
{
	this->muted = muted;
}

void GameScene::Advance(Util::OS::timestamp_t tick)
{
	SUPER::Advance(tick);

	if (!finishedLoading) return;

	session->Process();

	auto mainChar = session->GetPlayer(0)->GetMainCharacter();

	// Update HUD state last, after game state is settled for this frame.
	for (auto &viewport : viewports) {
		viewport.hud->Advance(tick);
	}
}

void GameScene::Layout()
{
	SUPER::Layout();
	LayoutViewports();
}

void GameScene::PrepareRender()
{
	SUPER::PrepareRender();

	if (!finishedLoading) return;

	for (auto &viewport : viewports) {
		viewport.hud->PrepareRender();
	}
}

void GameScene::Render()
{
	if (!finishedLoading) return;

	auto cfg = Config::GetInstance();
	MR_SimulationTime simTime = session->GetSimulationTime();

	{
		VideoServices::VideoBuffer *videoBuf = &display.GetLegacyDisplay();
		VideoServices::VideoBuffer::Lock lock(*videoBuf);

		int i = 0;
		for (auto &viewport : viewports) {
			viewport.observer->RenderNormalDisplay(videoBuf, session,
				session->GetPlayer(i++)->GetMainCharacter(),
				simTime, session->GetBackImage());
		}
	}

	if (cfg->runtime.enableHud) {
		for (auto &viewport : viewports) {
			if (viewport.hud->IsVisible()) {
				viewport.hud->Render();
			}
		}
	}

	// Trigger sounds.
	if (!muted) {
		int i = 0;
		for (auto &viewport : viewports) {
			viewport.observer->PlaySounds(session->GetCurrentLevel(),
				session->GetPlayer(i++)->GetMainCharacter());
		}
		VideoServices::SoundServer::ApplyContinuousPlay();
	}
}

/**
 * Redefine the bounds of each viewport based on the number of connected local
 * players.
 */
void GameScene::LayoutViewports()
{
	using Cell = Display::HudCell;

	// There will be no viewports until the scene has finished loading.
	if (viewports.empty()) return;

	HR_LOG(info) << "There are " << viewports.size() << " viewports!";

	switch (viewports.size()) {
		case 1:
			viewports[0].SetCell(Cell::FILL);
			break;
		case 2:
			viewports[0].SetCell(Cell::N);
			viewports[1].SetCell(Cell::S);
			break;
		case 3:
			viewports[0].SetCell(Cell::N);
			viewports[1].SetCell(Cell::SW);
			viewports[2].SetCell(Cell::SE);
			break;
		case 4:
			viewports[0].SetCell(Cell::NW);
			viewports[1].SetCell(Cell::NE);
			viewports[2].SetCell(Cell::SW);
			viewports[3].SetCell(Cell::SE);
			break;
		default:
			throw UnimplementedExn("Unhandled number of viewports: " +
				boost::lexical_cast<std::string>(viewports.size()));
	}
}

void GameScene::OnRaceFinish()
{
	//TODO: Currently assuming only one player, so we go directly from PLAYING
	//      to DONE (AdvancePhase will ensure that the POSTGAME event is fired).
	session->AdvancePhase(ClientSession::Phase::DONE);
	director.GetSessionChangedSignal()(nullptr);
}

}  // namespace HoverScript
}  // namespace Client
