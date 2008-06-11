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

#include "../Util/RecordFile.h"

enum MR_TrackAvail {
   eTrackAvail,
   eTrackNotFound
};


BOOL MR_SelectTrack(HWND pParentWindow, CString& pTrackFile, int& pNbLap, BOOL& pAllowWeapons);

// pFile name must contains no path and no extension
MR_RecordFile *MR_TrackOpen(HWND pWindow, const char* pFileName);

MR_TrackAvail MR_GetTrackAvail(const char* pFileName);

#endif
