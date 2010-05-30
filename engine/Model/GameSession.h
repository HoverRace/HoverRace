// GameSession.h
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
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

#ifndef GAME_SESSION_H
#define GAME_SESSION_H

// Includes
#include "Level.h"
#include "ContactEffect.h"
#include "../Parcel/RecordFile.h"

#ifdef MR_ENGINE
#define MR_DllDeclare   __declspec(dllexport)
#else
#define MR_DllDeclare   __declspec(dllimport)
#endif

// Class Declaration
class MR_GameSession
{
	private:
		BOOL mAllowRendering;
		int mCurrentLevelNumber;

		std::string mTitle;
		HoverRace::Parcel::RecordFile *mCurrentMazeFile;
		MR_Level *mCurrentLevel;

		MR_SimulationTime mSimulationTime;		  // Time simulated since the session start
		DWORD mLastSimulateCallTime;			  // Time in ms obtainend by timeGetTime

		BOOL LoadLevel(int pLevelIndex, char pGameOpts);
		void Clean();							  // Clean up before destruction or clean-up

		void SimulateFreeElems(MR_SimulationTime pDuration);
		int SimulateOneFreeElem(MR_SimulationTime pTimeToSimulate, MR_FreeElementHandle pElementHandle, int pRoom);
		void SimulateSurfaceElems(MR_SimulationTime pDuration);

		// SimulateFreeElem sub-functions
		void ComputeShapeContactEffects(int pCurrentRoom, MR_FreeElement *pActor, const MR_RoomContactSpec &pLastSpec, MR_FastArrayBase<int> *pVisitedRooms, int pMaxDepth, MR_SimulationTime pDuration);

	public:
		MR_DllDeclare MR_GameSession(BOOL pAllowRendering = FALSE);
		MR_DllDeclare ~MR_GameSession();

		MR_DllDeclare BOOL LoadNew(const char *pTitle, HoverRace::Parcel::RecordFile *pMazeFile, char pGameOpts);

		MR_DllDeclare void SetSimulationTime(MR_SimulationTime);
		MR_DllDeclare MR_SimulationTime GetSimulationTime() const;
		MR_DllDeclare void Simulate();
		MR_DllDeclare void SimulateLateElement(MR_FreeElementHandle pElement, MR_SimulationTime pDuration, int pRoom);

		MR_DllDeclare MR_Level *GetCurrentLevel();
		MR_DllDeclare const char *GetTitle() const;
		MR_DllDeclare HoverRace::Parcel::RecordFile *GetCurrentMazeFile();
};

#undef MR_DllDeclare
#endif