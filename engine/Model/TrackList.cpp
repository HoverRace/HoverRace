
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

#include "../Parcel/TrackBundle.h"
#include "../Util/OS.h"

#include "TrackList.h"

using namespace HoverRace::Parcel;
using HoverRace::Util::OS;

namespace HoverRace {
namespace Model {

TrackList::TrackList()
{
	tracks.reserve(1024);
}

void TrackList::Clear()
{
	if (!tracks.empty()) {
		tracks.clear();
	}
}

void TrackList::Reload(Parcel::TrackBundlePtr trackBundle)
{
	Clear();

	BOOST_FOREACH(const OS::dirEnt_t &ent, *trackBundle) {
		std::wstring s = boost::lexical_cast<std::wstring>(ent);
#		ifdef _WIN32
			OutputDebugStringW(s.c_str());
			OutputDebugStringW(L"\n");
#		endif
	}
}

}  // namespace Model
}  // namespace HoverRace
