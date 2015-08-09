
// ElemHeap.cpp
//
// Copyright (c) 2015 Michael Imamura.
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

#include "../Parcel/ObjStream.h"
#include "../Util/DllObjectFactory.h"
#include "MazeElement.h"

#include "ElemHeap.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Model {

void ElemHeap::StreamIn(Parcel::ObjStream &archive)
{
	assert(!archive.IsWriting());

	std::shared_ptr<FreeElement> newElem;

	elems.clear();

	do {
		ObjectFromFactory::SerializeShared<FreeElement>(archive, newElem);
		if (newElem) {
			newElem->mPosition.Serialize(archive);
			archive >> newElem->mOrientation;
			elems.push_back(newElem);
		}
	} while (newElem);
}

void ElemHeap::StreamOut(Parcel::ObjStream &archive) const
{
	assert(archive.IsWriting());

	static std::shared_ptr<ObjectFromFactory> NULL_OBJ;

	for (auto &elem : elems) {
		//TODO: Extend const-safety to SerializeShared.
		auto melem = std::const_pointer_cast<FreeElement>(elem);
		ObjectFromFactory::SerializeShared<FreeElement>(archive, melem);
		elem->mPosition.Serialize(archive);
		archive << elem->mOrientation;
	}
	ObjectFromFactory::SerializeShared<ObjectFromFactory>(archive, NULL_OBJ);
}

}  // namespace Model
}  // namespace HoverRace
