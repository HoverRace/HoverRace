
// TrackList.cpp
// Sorted list of track headers.
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

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include "../Parcel/ObjStream.h"
#include "../Parcel/TrackBundle.h"
#include "../Util/Str.h"
#include "../Util/OS.h"

#include "TrackList.h"

using namespace HoverRace::Parcel;
using HoverRace::Util::OS;
namespace Str = HoverRace::Util::Str;

namespace HoverRace {
namespace Model {

namespace {
	inline bool NameCmpFunc(const TrackEntry *ent1, const TrackEntry *ent2)
	{
		return ent1->name < ent2->name;
	}
	inline bool NameEqFunc(const TrackEntry *ent1, const TrackEntry *ent2)
	{
		return ent1->name == ent2->name;
	}
	inline bool NaturalCmpFunc(const TrackEntry *ent1, const TrackEntry *ent2)
	{
		return *ent1 < *ent2;
	}
}

TrackList::TrackList()
{
	tracks.reserve(1024);
}

/**
 * Clear the list of available tracks.
 */
void TrackList::Clear()
{
	if (!tracks.empty()) {
		tracks.clear();
	}
	if (!sorted.empty()) {
		sorted.clear();
	}
}

/**
 * Load the list of available tracks from the track bundle.
 * Any previously-loaded list is cleared.
 * @param trackBundle The track bundle (may not be @c NULL).
 */
void TrackList::Reload(Parcel::TrackBundlePtr trackBundle)
{
	Clear();

	BOOST_FOREACH(const OS::dirEnt_t &ent, *trackBundle) {
		std::string name(Str::PU(ent.path().filename().c_str()));
		try {
			Model::TrackEntryPtr trackEnt = trackBundle->OpenTrackEntry(name);
			if (trackEnt.get() != NULL) {
				tracks.push_back(*trackEnt);
#				ifdef _DEBUG
					tracks.back().path = ent.path();
#				endif
			}
		}
		catch (Parcel::ObjStreamExn &ex) {
			//TODO: Proper logging.
#			ifdef _WIN32
				OutputDebugString(ex.what());
				OutputDebugString("\n");
#			endif
			// Ignore this bad track and continue.
		}
	}

	sorted.reserve(tracks.size());
	BOOST_FOREACH(TrackEntry &ent, tracks) {
		sorted.push_back(&ent);
	}

	// Use a stable sort so that if there are multiple entries with the
	// same name, then the bundle priority order will be preserved.
	// Then, when we remove duplicates, the lower-priority entries will be
	// the ones which are removed.
	std::stable_sort(sorted.begin(), sorted.end(), NameCmpFunc);
	sorted.erase(std::unique(sorted.begin(), sorted.end(), NameEqFunc), sorted.end());

	// Re-sort using the natural ordering.
	std::sort(sorted.begin(), sorted.end(), NaturalCmpFunc);
}

}  // namespace Model
}  // namespace HoverRace
