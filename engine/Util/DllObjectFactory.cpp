
// DllObjectFactory.cpp
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
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
//

#include <map>

#include "../ObjFac1/ObjFac1.h"
#include "../Parcel/ObjStream.h"
#include "../Util/Log.h"
#include "../Exception.h"

#include "DllObjectFactory.h"

using HoverRace::ObjFac1::ObjFac1;
using HoverRace::Parcel::ObjStream;

namespace HoverRace {
namespace Util {

namespace {

std::unique_ptr<ObjFac1::ObjFac1> dll;

/**
 * Lazy-initialize the object factory.
 * The option of choosing which DLL has been deprecated.
 * @param dllId The DLL ID (must be 1).
 * @return The object factory.
 */
ObjFac1::ObjFac1 &GetDll(MR_UInt16 dllId)
{
	assert(dllId == 1);

	if (!dll) {
		dll.reset(new ObjFac1::ObjFac1());
	}

	return *dll;
}

}  // namespace

void DllObjectFactory::Init()
{
	// Nothing to do; will be lazily-initialized.
}

void DllObjectFactory::Clean()
{
	dll.reset();
}

ObjectFromFactory *DllObjectFactory::CreateObject(const ObjectFromFactoryId &pId)
{
	return GetDll(pId.mDllId).GetObject(pId.mClassId);
}

/**
 * Gets the resource library for an object factory.
 * @param dllId Identifier for the DLL.
 * @return The resource library.
 */
ObjFacTools::ResourceLib &DllObjectFactory::GetResourceLib(MR_UInt16 dllId)
{
	return *(GetDll(dllId).GetResourceLib());
}

void ObjectFromFactory::ThrowUnexpected(const ObjectFromFactoryId &oid)
{
	throw Parcel::ObjStreamExn(boost::str(boost::format(
		"Unexpected object type in stream: {%d, %d}") %
		oid.mDllId % oid.mClassId));
}

// ObjectFromFactoryId
void ObjectFromFactoryId::Serialize(ObjStream &pArchive)
{
	if(pArchive.IsWriting()) {
		pArchive << mDllId << mClassId;
	}
	else {
		pArchive >> mDllId >> mClassId;
	}
}

}  // namespace Util
}  // namespace HoverRace
