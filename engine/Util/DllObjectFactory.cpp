
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

// Base class for object factory "DLLs".
// (Originally, these were real DLLs which were loaded at runtime).
class FactoryDll
{
public:
	// Initialisation
	FactoryDll() { }
	virtual ~FactoryDll() { }

	virtual ObjectFromFactory* GetObject(int classId) const = 0;
	virtual ObjFacTools::ResourceLib &GetResourceLib() const = 0;
};

// Using ObjFac1 (read from package file).
class PackageFactoryDll : public FactoryDll
{
	using SUPER = FactoryDll;

public:
	PackageFactoryDll();
	virtual ~PackageFactoryDll();

	ObjectFromFactory* GetObject(int classId) const override;

	ObjFacTools::ResourceLib &GetResourceLib() const override
	{
		return *(package->GetResourceLib());
	}

private:
	ObjFac1::ObjFac1 *package;
};

// Local functions declarations
static FactoryDll *GetDll(MR_UInt16 pDllId);

namespace {

// Module variables
using gsDllList_t = std::map<int, std::shared_ptr<FactoryDll>>;
gsDllList_t gsDllList;

}  // namespace

void DllObjectFactory::Init()
{
	// Nothing to do
}

void DllObjectFactory::Clean()
{
	gsDllList.clear();
}

ObjectFromFactory *DllObjectFactory::CreateObject(const ObjectFromFactoryId &pId)
{
	ObjectFromFactory *lReturnValue;

	FactoryDll *lDllPtr = GetDll(pId.mDllId);

	lReturnValue = lDllPtr->GetObject(pId.mClassId);

	return lReturnValue;
}

/**
 * Gets the resource library for an object factory.
 * @param dllId Identifier for the DLL.
 * @return The resource library.
 */
ObjFacTools::ResourceLib &DllObjectFactory::GetResourceLib(MR_UInt16 dllId)
{
	return GetDll(dllId)->GetResourceLib();
}

/**
 * Get a handle to the factory DLL.
 * The option of choosing which DLL has been deprecated.
 */
FactoryDll *GetDll(MR_UInt16 pDllId)
{
	FactoryDll *lDllPtr;

	ASSERT(pDllId != 0);						  // Number 0 is reserved for NULL entry

	// Create the factory DLL if it doesn't exist already.
	gsDllList_t::iterator iter = gsDllList.find(pDllId);
	if (iter == gsDllList.end()) {
		ASSERT(pDllId == 1);  // Number 1 is the package (ObjFac1) by convention from original code.
		lDllPtr =
			gsDllList.emplace(pDllId,
				std::make_shared<PackageFactoryDll>()).first->second.get();
	}
	else {
		return iter->second.get();
	}

	return lDllPtr;
}

void ObjectFromFactory::SerializePtr(ObjStream &pArchive, ObjectFromFactory *&pPtr)
{
	ObjectFromFactoryId lId = { 0, 0 };

	if(pArchive.IsWriting()) {
		if(pPtr != NULL) {
			lId = pPtr->mId;
		}

		lId.Serialize(pArchive);

		if(pPtr != NULL) {
			pPtr->Serialize(pArchive);
		}
	}
	else {
		lId.Serialize(pArchive);

		if(lId.mDllId == 0) {
			pPtr = NULL;
		}
		else {
			pPtr = DllObjectFactory::CreateObject(lId);
			pPtr->Serialize(pArchive);
		}
	}
}

void ObjectFromFactory::Serialize(ObjStream&)
{
	// Nothing to serialize at that point.
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

PackageFactoryDll::PackageFactoryDll() :
	SUPER(),
	package(new ObjFac1::ObjFac1())
{
}

PackageFactoryDll::~PackageFactoryDll()
{
	delete package;
}

ObjectFromFactory *PackageFactoryDll::GetObject(int classId) const
{
	return package->GetObject(classId);
}

}  // namespace Util
}  // namespace HoverRace
