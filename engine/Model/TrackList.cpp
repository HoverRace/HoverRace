
// TrackList.cpp
//
// Copyright (c) 2010, 2013, 2015 Michael Imamura.
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

#include <boost/lexical_cast.hpp>

#include "../Parcel/ObjStream.h"
#include "../Parcel/TrackBundle.h"
#include "../Util/Str.h"
#include "../Util/Log.h"
#include "../Util/OS.h"

#include "TrackList.h"

using namespace HoverRace::Parcel;
using namespace HoverRace::Util;

namespace HoverRace {
namespace Model {

namespace {

inline bool NameCmpFunc(
	const std::shared_ptr<TrackEntry> &ent1,
	const std::shared_ptr<TrackEntry> &ent2)
{
	return ent1->name < ent2->name;
}
inline bool NameEqFunc(
	const std::shared_ptr<TrackEntry> &ent1,
	const std::shared_ptr<TrackEntry> &ent2)
{
	return ent1->name == ent2->name;
}
inline bool NaturalCmpFunc(
	const std::shared_ptr<TrackEntry> &ent1,
	const std::shared_ptr<TrackEntry> &ent2)
{
	return *ent1 < *ent2;
}

}  // namespace

TrackList::TrackList()
{
	tracks.reserve(1024);
}

/**
 * Load the list of available tracks from the track bundle.
 * Any previously-loaded list is cleared.
 * @param trackBundle The track bundle (may not be @c nullptr).
 */
void TrackList::Reload(std::shared_ptr<Parcel::TrackBundle> trackBundle)
{
	Clear();

	for (const OS::dirEnt_t &ent : *trackBundle) {
		std::string name((const char*)Str::PU(ent.path().filename().c_str()));
		try {
			auto trackEnt = trackBundle->OpenTrackEntry(name);
			if (trackEnt) {
				tracks.emplace_back(trackEnt);
#				ifdef _DEBUG
					tracks.back()->path = ent.path();
#				endif
			}
		}
		catch (Parcel::ObjStreamExn &ex) {
			// Ignore this bad track and continue.
			Log::Warn("Skipping invalid track: %s: %s", name.c_str(), ex.what());
		}
	}

	// Use a stable sort so that if there are multiple entries with the
	// same name, then the bundle priority order will be preserved.
	// Then, when we remove duplicates, the lower-priority entries will be
	// the ones which are removed.
	std::stable_sort(tracks.begin(), tracks.end(), NameCmpFunc);
	tracks.erase(
		std::unique(tracks.begin(), tracks.end(), NameEqFunc),
		tracks.end());

	// Re-sort using the natural ordering.
	std::sort(tracks.begin(), tracks.end(), NaturalCmpFunc);
}

}  // namespace Model
}  // namespace HoverRace
