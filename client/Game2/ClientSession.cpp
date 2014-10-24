// ClientSession.cpp
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
// Copyright (c) 2013, 2014 Michael Imamura.
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
//

#include "../StdAfx.h"

#include <boost/thread/locks.hpp>

#include "../../engine/MainCharacter/MainCharacter.h"
#include "../../engine/Model/Track.h"
#include "../../engine/Model/TrackFileCommon.h"
#include "../../engine/Player/Player.h"
#include "../../engine/VideoServices/VideoBuffer.h"
#include "../../engine/Util/Clock.h"
#include "../../engine/Util/Duration.h"
#include "../../engine/Util/Log.h"

#include "HoverScript/MetaSession.h"
#include "HoverScript/TrackPeer.h"
#include "Rules.h"

#include "ClientSession.h"

using namespace HoverRace::Parcel;
using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

namespace {
	using Phase = ClientSession::Phase;

	/**
	 * Determine next phase for a given phase.
	 * @param phase The phase.
	 * @return The next phase, if there is one.
	 */
	Phase NextPhase(Phase phase)
	{
		if (phase == Phase::DONE) {
			return Phase::DONE;
		}
		else {
			return static_cast<Phase>(static_cast<int>(phase) + 1);
		}
	}

	MainCharacter::MainCharacter *GetMainChar(Player::Player *player)
	{
		return player ? player->GetMainCharacter() : nullptr;
	}
}

ClientSession::ClientSession(std::shared_ptr<Rules> rules) :
	phase(Phase::INIT),
	mSession(true),
	clock(std::make_shared<Util::Clock>()),
	rules(std::move(rules)),
	mBackImage(nullptr)
{
}

ClientSession::~ClientSession()
{
	for (auto &player : players) {
		if (player) {
			player->DetachMainCharacter();
		}
	}

	delete[] mBackImage;
}

/**
 * Advance to the next phase.
 * @return @c true if the new phase was set successfully, @c false otherwise.
 */
bool ClientSession::AdvancePhase()
{
	return AdvancePhase(NextPhase(phase));
}

/**
 * Advance the current phase.
 *
 * The new phase must be equal to or later than the current phase.  If any
 * phases are skipped, then then the events for the skipped phases will still
 * be called.
 *
 * @param nextPhase The requested next phase.
 * @return @c true if the new phase was set successfully, @c false otherwise.
 */
bool ClientSession::AdvancePhase(Phase nextPhase)
{
	if (phase < nextPhase) {
		do {
			if (countdown) {
				countdownConn.disconnect();
				countdown.reset();
			}

			phase = NextPhase(phase);
			switch (phase) {
				case Phase::PREGAME: meta->OnPregame(); break;
				case Phase::PLAYING: meta->OnPlaying(); break;
				case Phase::POSTGAME: meta->OnPostgame(); break;
				case Phase::DONE: meta->OnDone(); break;
				default:
					Log::Warn("No MetaSession event for phase: %d", phase);
			}
		} while (phase < nextPhase);
		return true;
	}
	else if (nextPhase < phase) {
		Log::Error("Attempted to go backwards in phase!");
	}
	return false;
}

void ClientSession::SetMeta(std::shared_ptr<HoverScript::MetaSession> meta)
{
	this->meta = std::move(meta);
}

void ClientSession::Process()
{
	if (countdown) {
		countdown->Advance();
	}

	auto simTime = mSession.GetSimulationTime();
	if (clock->GetTime() == 0) {
		if (simTime > 0) {
			// Leaving pregame.
			clock->SetTime(simTime);
		}
	}
	else {
		clock->Advance();
	}

	UpdateCharacterSimulationTimes();
	mSession.Simulate();
}

void ClientSession::ReadLevelAttrib(Parcel::RecordFilePtr pRecordFile, VideoServices::VideoBuffer * pVideo)
{
	// Read level background palette
	if((pVideo != NULL) && (pRecordFile->GetNbRecords() >= 3)) {
		pRecordFile->SelectRecord(2);

		{
			ObjStreamPtr archivePtr(pRecordFile->StreamIn());
			ObjStream &lArchive = *archivePtr;

			int lImageType;

			lArchive >> lImageType;

			if(lImageType == MR_RAWBITMAP) {
				auto lPalette = std::unique_ptr<MR_UInt8[]>(new MR_UInt8[MR_BACK_COLORS * 3]);

				if(mBackImage == NULL) {
					mBackImage = new MR_UInt8[MR_BACK_X_RES * MR_BACK_Y_RES];
				}

				lArchive.Read(lPalette.get(), MR_BACK_COLORS * 3);
				lArchive.Read(mBackImage, MR_BACK_X_RES * MR_BACK_Y_RES);

				pVideo->SetBackgroundPalette(lPalette);
			}
		}
	}
}

bool ClientSession::LoadNew(const char *pTitle, Script::Core *scripting,
                            std::shared_ptr<Model::Track> track,
                            VideoServices::VideoBuffer *pVideo)
{
	bool retv = mSession.LoadNew(pTitle, track, rules->GetGameOpts());

	if (retv) {
		ReadLevelAttrib(track->GetRecordFile(), pVideo);
		trackPeer = std::make_shared<HoverScript::TrackPeer>(scripting, track);
	}

	return retv;
}

const MR_UInt8 *ClientSession::GetBackImage() const
{
	return mBackImage;
}

/**
 * Attach a player to the session.
 * @param i Zero-based index of the player slot.
 * @param player The player (may not be @c nullptr).
 */
void ClientSession::AttachPlayer(int i, std::shared_ptr<Player::Player> player)
{
	ASSERT(player);
	ASSERT(i >= 0 && i < MAX_PLAYERS);

	auto &playerSlot = players[static_cast<size_t>(i)];
	if (playerSlot) {
		playerSlot->DetachMainCharacter();
	}
	playerSlot = std::move(player);

	Model::Level *curLevel = mSession.GetCurrentLevel();
	ASSERT(curLevel);

	MainCharacter::MainCharacter *ch =
		MainCharacter::MainCharacter::New(i, rules->GetGameOpts());
	playerSlot->AttachMainCharacter(ch);

	int startingRoom = curLevel->GetStartingRoom(i);
	ch->mRoom = startingRoom;
	ch->mPosition = curLevel->GetStartingPos(i);
	ch->SetOrientation(curLevel->GetStartingOrientation(i));
	ch->SetHoverId(i);

	curLevel->InsertElement(ch, startingRoom);
}

/**
 * Set a countdown timer to the next phase.
 * @param duration The duration (must be positive).
 */
void ClientSession::CountdownToNextPhase(const Util::Duration &duration)
{
	if (countdown) {
		// Cancel the alarm.
		// We can't depend on the countdown clock going away, since a
		// shared_ptr to it might still be held by something else.
		countdownConn.disconnect();
	}
	countdown = std::make_shared<Util::Clock>(-duration);
	countdownConn = countdown->At(0, [&]() {
		AdvancePhase();
	});

	// Update the simulation time so we play the pregame animation.
	if (phase == Phase::PREGAME) {
		SetSimulationTime(-static_cast<MR_SimulationTime>(duration.ToMs()));
	}
}

void ClientSession::SetSimulationTime(MR_SimulationTime pTime)
{
	//TODO: Use the clock.
	mSession.SetSimulationTime(pTime);
}

MR_SimulationTime ClientSession::GetSimulationTime() const
{
	//TODO: Use the clock.
	return mSession.GetSimulationTime();
}

void ClientSession::UpdateCharacterSimulationTimes()
{
	for (auto &player : players) {
		if (player) {
			if (auto mainCharacter = player->GetMainCharacter()) {
				mainCharacter->SetSimulationTime(mSession.GetSimulationTime());
			}
		}
	}
}

const Model::Level *ClientSession::GetCurrentLevel() const
{
	return mSession.GetCurrentLevel();
}

int ClientSession::ResultAvaillable() const
{
	return 0;
}

/** @deprecated */
void ClientSession::GetResult(int, const char *&pPlayerName, int&, BOOL&,
                              int&, MR_SimulationTime&,
                              MR_SimulationTime&) const
{
	pPlayerName = "?";
	ASSERT(FALSE);
}

/** @deprecated */
void ClientSession::GetHitResult(int, const char *&pPlayerName,
                                 int&, BOOL&, int&, int&) const
{
	pPlayerName = "?";
	ASSERT(FALSE);
}

int ClientSession::GetNbPlayers() const
{
	int retv = 0;

	for (auto &player : players) {
		if (player && player->GetMainCharacter()) {
			++retv;
		}
	}

	return retv;
}

int ClientSession::GetRank(const MainCharacter::MainCharacter *pPlayer) const
{
	int lReturnValue = 1;

	MainCharacter::MainCharacter *mMainCharacter1 = GetMainChar(players[0].get());
	MainCharacter::MainCharacter *mMainCharacter2 = GetMainChar(players[1].get());
	MainCharacter::MainCharacter *mMainCharacter3 = GetMainChar(players[2].get());
	MainCharacter::MainCharacter *mMainCharacter4 = GetMainChar(players[3].get());

	if(mMainCharacter1 != NULL) {
		if(pPlayer == mMainCharacter1) {
			if(mMainCharacter2->HasFinish()) {
				if(mMainCharacter2->GetTotalTime() < mMainCharacter1->GetTotalTime()) {
					lReturnValue++;
				}
			}
			if(mMainCharacter3->HasFinish()) {
				if(mMainCharacter3->GetTotalTime() < mMainCharacter1->GetTotalTime()) {
					lReturnValue++;
				}
			}
			if(mMainCharacter4->HasFinish()) {
				if(mMainCharacter4->GetTotalTime() < mMainCharacter1->GetTotalTime()) {
					lReturnValue++;
				}
			}
		}
		if(pPlayer == mMainCharacter2) {
			lReturnValue = 1;

			if(mMainCharacter1->HasFinish()) {
				if(mMainCharacter1->GetTotalTime() < mMainCharacter2->GetTotalTime()) {
					lReturnValue++;
				}
			}
			if(mMainCharacter3->HasFinish()) {
				if(mMainCharacter3->GetTotalTime() < mMainCharacter2->GetTotalTime()) {
					lReturnValue++;
				}
			}
			if(mMainCharacter4->HasFinish()) {
				if(mMainCharacter4->GetTotalTime() < mMainCharacter2->GetTotalTime()) {
					lReturnValue++;
				}
			}
		}
		if(pPlayer == mMainCharacter3) {
			lReturnValue = 1;

			if(mMainCharacter1->HasFinish()) {
				if(mMainCharacter1->GetTotalTime() < mMainCharacter3->GetTotalTime()) {
					lReturnValue++;
				}
			}
			if(mMainCharacter2->HasFinish()) {
				if(mMainCharacter2->GetTotalTime() < mMainCharacter3->GetTotalTime()) {
					lReturnValue++;
				}
			}
			if(mMainCharacter4->HasFinish()) {
				if(mMainCharacter4->GetTotalTime() < mMainCharacter3->GetTotalTime()) {
					lReturnValue++;
				}
			}
		}
		if(pPlayer == mMainCharacter4) {
			lReturnValue = 1;

			if(mMainCharacter1->HasFinish()) {
				if(mMainCharacter1->GetTotalTime() < mMainCharacter4->GetTotalTime()) {
					lReturnValue++;
				}
			}
			if(mMainCharacter2->HasFinish()) {
				if(mMainCharacter2->GetTotalTime() < mMainCharacter4->GetTotalTime()) {
					lReturnValue++;
				}
			}
			if(mMainCharacter3->HasFinish()) {
				if(mMainCharacter3->GetTotalTime() < mMainCharacter4->GetTotalTime()) {
					lReturnValue++;
				}
			}
		}
	}
	return lReturnValue;
}

void ClientSession::AddMessageKey(char /*pKey */ )
{

}

void ClientSession::GetCurrentMessage(char *pDest) const
{
	pDest[0] = 0;
}

BOOL ClientSession::GetMessageStack(int pLevel, char *pDest, int pExpiration) const
{
	BOOL lReturnValue = FALSE;

	if(pLevel < CHAT_MESSAGE_STACK) {
		boost::lock_guard<boost::mutex> lock(chatMutex);

		if(((mMessageStack[pLevel].mCreationTime + pExpiration) > time(NULL)) &&
			(mMessageStack[pLevel].mBuffer.length() > 0))
		{
			lReturnValue = TRUE;
			strcpy(pDest, mMessageStack[pLevel].mBuffer.c_str());
		}
	}

	return lReturnValue;
}

void ClientSession::AddMessage(const char *pMessage)
{
	boost::lock_guard<boost::mutex> lock(chatMutex);

	for(int lCounter = CHAT_MESSAGE_STACK - 1; lCounter > 0; lCounter--) {
		mMessageStack[lCounter] = mMessageStack[lCounter - 1];
	}

	mMessageStack[0].mCreationTime = time(NULL);

	mMessageStack[0].mBuffer = VideoServices::Ascii2Simple(pMessage);
}

}  // namespace Client
}  // namespace HoverRace
