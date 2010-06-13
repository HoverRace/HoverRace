
// Track.h
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

#pragma once

#include "TrackEntry.h"

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
	namespace Parcel {
		class RecordFile;
		typedef boost::shared_ptr<RecordFile> RecordFilePtr;
	}
}

namespace HoverRace {
namespace Model {

/**
 * A track level.
 * @author Michael Imamura
 */
class MR_DllDeclare Track
{
	private:
		Track() { }
	public:
		Track(Parcel::RecordFilePtr recFile);
		~Track();

		//HACK: Temporary until track data loading moved into this class.
		Parcel::RecordFilePtr GetRecordFile() const { return recFile; }

	private:
		Parcel::RecordFilePtr recFile;
		TrackEntry header;
};
typedef boost::shared_ptr<Track> TrackPtr;

}  // namespace Model
}  // namespace HoverRace

#undef MR_DllDeclare
