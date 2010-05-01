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

namespace HoverRace {
	namespace Parcel {
		class RecordFile;
	}
}

// for gsGameOpts
#define OPT_ALLOW_WEAPONS	0x40
#define OPT_ALLOW_MINES		0x20
#define OPT_ALLOW_CANS		0x10
#define OPT_ALLOW_BASIC		0x08
#define OPT_ALLOW_BI		0x02
#define OPT_ALLOW_CX		0x04
#define OPT_ALLOW_EON		0x01

enum MR_TrackAvail
{
	eTrackAvail,
	eTrackNotFound
};

bool MR_SelectTrack(HWND pParentWindow, std::string &pTrackFile, int &pNbLap, char &pGameOpts);

// pFile name must contains no path and no extension
HoverRace::Parcel::RecordFile *MR_TrackOpen(HWND pWindow, const char *pFileName);

MR_TrackAvail MR_GetTrackAvail(const char *pFileName);

#endif
