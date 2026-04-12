// TrackSelect.h
//
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

#ifndef TRACK_SELECT_H
#define TRACK_SELECT_H

#include <string>

#include "../Util/RecordFile.h"
#include "GameRules.h"

enum MR_TrackAvail
{
	eTrackAvail,
	eTrackNotFound
};

unsigned MR_GetDefaultAllowedCraftMask();
unsigned MR_NormalizeAllowedCraftMask(unsigned pAllowedCraftMask);
bool MR_HasAllowedCraft(unsigned pAllowedCraftMask);
bool MR_IsCraftAllowed(unsigned pAllowedCraftMask, int pCraftId);
int MR_GetFirstAllowedCraft(unsigned pAllowedCraftMask);
int MR_GetNextAllowedCraft(unsigned pAllowedCraftMask, int pCurrentCraftId,
	int pDirection);
std::string MR_FormatAllowedCraftMask(unsigned pAllowedCraftMask);
std::string MR_FormatAllowedCraftDisplayMask(unsigned pAllowedCraftMask);
std::string MR_FormatPowerupDisplay(bool pAllowWeapons, bool pAllowCans,
	bool pAllowMines);
unsigned MR_ParseAllowedCraftMask(const char *pAllowedCrafts);

bool MR_SelectTrack(HWND pParentWindow, std::string &pTrackFile, int &pNbLap,
	bool &pAllowWeapons, bool &pAllowCans, bool &pAllowMines,
	unsigned &pAllowedCraftMask, MR_GameRuleSettings &pGameRuleSettings,
	bool pPracticeMode = false);

// pFile name must contains no path and no extension
MR_RecordFile *MR_TrackOpen(HWND pWindow, const char *pFileName);

MR_TrackAvail MR_GetTrackAvail(const char *pFileName);

#endif
