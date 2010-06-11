
// TrackBundle.h
// A source of track parcels.
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

#include "Bundle.h"

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
	namespace Model {
		class Track;
		typedef boost::shared_ptr<Track> TrackPtr;
	}
}

enum MR_TrackAvail
{
	eTrackAvail,
	eTrackNotFound
};

namespace HoverRace {
namespace Parcel {

/**
 * A source of track parcels.
 * @author Michael Imamura
 */
class MR_DllDeclare TrackBundle : public Bundle
{
	typedef Bundle SUPER;
	public:
		TrackBundle(const Util::OS::path_t &dir, BundlePtr subBundle=BundlePtr());
		virtual ~TrackBundle();

		virtual RecordFilePtr OpenParcel(const std::string &name, bool writing=false) const;

		Model::TrackPtr OpenTrack(const std::string &name) const;

		MR_TrackAvail CheckAvail(const std::string &name) const;
};
typedef boost::shared_ptr<TrackBundle> TrackBundlePtr;

}  // namespace Parcel
}  // namespace HoverRace

#undef MR_DllDeclare
