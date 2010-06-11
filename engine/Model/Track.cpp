
// Track.cpp
// A track.
//
// Copyright (c) 2010 Michael Imamura.
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

#include "../Parcel/RecordFile.h"

#include "Track.h"

namespace HoverRace {
namespace Model {

/**
 * Constructor.
 * @param recFile The record file to load the track from (may not be @c NULL).
 * @throw Parcel::ObjStreamExn
 */
Track::Track(Parcel::RecordFilePtr recFile) :
	recFile(recFile)
{
	recFile->SelectRecord(0);
	header.Serialize(*recFile->StreamIn());
}

Track::~Track()
{
}

}  // namespace Model
}  // namespace HoverRace
