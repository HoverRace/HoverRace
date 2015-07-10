
// TrackBundle.h
//
// Copyright (c) 2010, 2013-2015 Michael Imamura.
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

#include "../Display/Res.h"

#include "Bundle.h"

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
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
		class TrackEntry;
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
	using SUPER = Bundle;
public:
	TrackBundle(const Util::OS::path_t &dir,
		std::shared_ptr<Bundle> subBundle = std::shared_ptr<Bundle>());
	virtual ~TrackBundle() { }

	std::shared_ptr<RecordFile> OpenParcel(const std::string &name,
		bool writing = false) const override;

	std::shared_ptr<Model::Track> OpenTrack(const std::string &name) const;
	std::shared_ptr<Model::Track> OpenTrack(
		const std::shared_ptr<const Model::TrackEntry> &entry) const;
	std::shared_ptr<Display::Res<Display::Texture>> LoadMap(
		std::shared_ptr<const Model::TrackEntry> entry) const;
	std::shared_ptr<Model::TrackEntry> OpenTrackEntry(
		const std::string &name) const;

	MR_TrackAvail CheckAvail(const std::string &name) const;
};
typedef std::shared_ptr<TrackBundle> TrackBundlePtr;

}  // namespace Parcel
}  // namespace HoverRace

#undef MR_DllDeclare
