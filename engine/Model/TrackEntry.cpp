
// TrackEntry.cpp
//
// Copyright (c) 2010, 2014, 2015 Michael Imamura.
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

#include "../Model/TrackFileCommon.h"
#include "../Parcel/ObjStream.h"
#include "../Util/InspectMapNode.h"
#include "../Exception.h"
#include "TrackFormatExn.h"

#include "TrackEntry.h"

namespace HoverRace {
namespace Model {

void TrackEntry::Serialize(Parcel::ObjStream &os)
{
	if (os.IsWriting()) {
		throw UnimplementedExn("TrackEntry::Serialize for writing");
	}
	else {
		MR_Int32 magicNumber;
		MR_Int32 version;

		os >> magicNumber;
		if (magicNumber != MR_MAGIC_TRACK_NUMBER)
			throw TrackFormatExn(boost::str(boost::format(
				"Bad magic number: 0x%08x") % magicNumber));

		os >> version;
		if (version != 1)
			throw TrackFormatExn(boost::str(boost::format(
				"Unknown track version: %d") % version));

		os >> description >> regMinor >> regMajor >> sortingIndex >> registrationMode;

		if (registrationMode == MR_FREE_TRACK) {
			os >> magicNumber;
			if (magicNumber != MR_MAGIC_TRACK_NUMBER)
				throw TrackFormatExn(boost::str(boost::format(
					"Bad magic number for free track: 0x%08x") % magicNumber));
		}
	}
}

void TrackEntry::Inspect(Util::InspectMapNode &node) const
{
	node.
		AddField("name", name).
		AddField("description", description).
		AddField("regMinor", regMinor).
		AddField("regMajor", regMajor).
		AddField("registrationMode",
			(registrationMode == MR_FREE_TRACK) ? "free" : "registered").
		AddField("sortingIndex", sortingIndex);
}

}  // namespace Model
}  // namespace HoverRace
