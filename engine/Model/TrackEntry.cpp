
// TrackEntry.cpp
// Track metadata.
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

#include <boost/algorithm/string/replace.hpp>

#include "../../compilers/MazeCompiler/TrackCommonStuff.h"

#include "../Parcel/ObjStream.h"

#include "TrackEntry.h"

namespace HoverRace {
namespace Model {

void TrackEntry::Serialize(Parcel::ObjStream &os)
{
	if (os.IsWriting()) {
		//TODO
	}
	else {
		MR_Int32 magicNumber;
		MR_Int32 version;

		os >> magicNumber;
		if (magicNumber != MR_MAGIC_TRACK_NUMBER)
			throw std::exception();  //TODO: Throw invalid format exception.

		os >> version;
		if (version != 1)
			throw std::exception();  //TODO: Throw invalid format exception.

		os >> description >> regMinor >> regMajor >> sortingIndex >> registrationMode;
#		ifdef _WIN32
			boost::algorithm::replace_all(description, "\n", "\r\n");
#		endif

		if (registrationMode == MR_FREE_TRACK) {
			os >> magicNumber;
			if (magicNumber != MR_MAGIC_TRACK_NUMBER)
				throw std::exception();  //TODO: Throw invalid format exception.
		}
	}
}

void TrackEntry::Inspect(Util::InspectMapNode &node) const
{
	node.
		AddField("filename", filename).
		AddField("description", description).
		AddField("regMinor", regMinor).
		AddField("regMajor", regMajor).
		AddField("registrationMode", (registrationMode == MR_FREE_TRACK) ? "free" : "registered").
		AddField("sortingIndex", sortingIndex);
}

}  // namespace Model
}  // namespace HoverRace
