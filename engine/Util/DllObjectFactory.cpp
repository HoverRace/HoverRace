
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
	FactoryDll();
	virtual ~FactoryDll();

	virtual ObjectFromFactory* GetObject(int classId) const = 0;
	virtual ObjFacTools::ResourceLib &GetResourceLib() const = 0;

public:
	bool mDynamic;
	int mRefCount;
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

// Using function pointer callback.
class LocalFactoryDll : public FactoryDll
{
	using SUPER = FactoryDll;

private:
	LocalFactoryDll() = delete;
public:
	LocalFactoryDll(DllObjectFactory::getObject_t getObject);
	virtual ~LocalFactoryDll();

	ObjectFromFactory* GetObject(int classId) const override;

	ObjFacTools::ResourceLib &GetResourceLib() const override
	{
		throw UnimplementedExn("LocalFactoryDll::GetResourceLib");
	}

private:
	DllObjectFactory::getObject_t getObject;
};

// Local functions declarations
static FactoryDll *GetDll(MR_UInt16 pDllId, BOOL pTrowOnError);

namespace {

// Module variables
using gsDllList_t = std::map<int, FactoryDll*>;
gsDllList_t gsDllList;

}  // namespace

// Module functions
void DllObjectFactory::Init()
{
	// Nothing to do
}

void DllObjectFactory::Clean(BOOL pOnlyDynamic)
{
	for (gsDllList_t::iterator iter = gsDllList.begin(); iter != gsDllList.end(); ) {
		FactoryDll* dllPtr = iter->second;

		if ((dllPtr->mRefCount <= 0) && (dllPtr->mDynamic || !pOnlyDynamic)) {
			gsDllList.erase(iter++);
			delete dllPtr;
		}
		else {
			++iter;
		}
	}
}

void DllObjectFactory::RegisterLocalDll(int pDllId, getObject_t pFunc)
{

	FactoryDll *lDllPtr;

	ASSERT(pDllId != 0);						  // Number 0 is reserved for NULL entry
	ASSERT(pDllId != 1);						  // Number 1 is reserved for ObjFac1
	ASSERT(pFunc != NULL);

	lDllPtr = new LocalFactoryDll(pFunc);

	bool inserted = gsDllList.emplace(pDllId, lDllPtr).second;
	ASSERT(inserted);
}

void DllObjectFactory::IncrementReferenceCount(int pDllId)
{
	gsDllList_t::iterator iter = gsDllList.find(pDllId);
	if (iter != gsDllList.end()) {
		iter->second->mRefCount++;
	}
	else {
		ASSERT(FALSE);							  // Dll not loaded
	}
}

void DllObjectFactory::DecrementReferenceCount(int pDllId)
{
	gsDllList_t::iterator iter = gsDllList.find(pDllId);
	if (iter != gsDllList.end()) {
		iter->second->mRefCount--;
	}
	else {
		ASSERT(FALSE);							  // Dll discarded
	}
}

ObjectFromFactory *DllObjectFactory::CreateObject(const ObjectFromFactoryId &pId)
{
	ObjectFromFactory *lReturnValue;

	FactoryDll *lDllPtr = GetDll(pId.mDllId, TRUE);

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
	return GetDll(dllId, TRUE)->GetResourceLib();
}

/**
 * Get a handle to the factory DLL.
 * The option of choosing which DLL has been deprecated.
 */
FactoryDll *GetDll(MR_UInt16 pDllId, BOOL)
{
	FactoryDll *lDllPtr;

	ASSERT(pDllId != 0);						  // Number 0 is reserved for NULL entry

	// Create the factory DLL if it doesn't exist already.
	gsDllList_t::iterator iter = gsDllList.find(pDllId);
	if (iter == gsDllList.end()) {
		ASSERT(pDllId == 1);  // Number 1 is the package (ObjFac1) by convention from original code.
		lDllPtr = new PackageFactoryDll();
		gsDllList.insert(gsDllList_t::value_type(pDllId, lDllPtr));
	}
	else {
		return iter->second;
	}

	return lDllPtr;
}

// ObjectFromFactory methods
ObjectFromFactory::ObjectFromFactory(const ObjectFromFactoryId &pId)
{
	mId = pId;

	// Increment this object dll reference count
	// This will prevent the Dll from being discarded
	if (mId.mDllId <= 1) {
		DllObjectFactory::IncrementReferenceCount(mId.mDllId);
	}
}

ObjectFromFactory::~ObjectFromFactory()
{

	// Decrement this object dll reference count
	// This will allow the dll to be freed if not needed anymore
	if (mId.mDllId <= 1) {
		DllObjectFactory::DecrementReferenceCount(mId.mDllId);
	}
}

const ObjectFromFactoryId &ObjectFromFactory::GetTypeId() const
{
	return mId;
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

// class FactoryDll methods
FactoryDll::FactoryDll() :
	mDynamic(false), mRefCount(0)
{
}

FactoryDll::~FactoryDll()
{
	ASSERT(mRefCount == 0);
}

PackageFactoryDll::PackageFactoryDll() :
	SUPER()
{
	mDynamic = true;
	package = new ObjFac1::ObjFac1();
}

PackageFactoryDll::~PackageFactoryDll()
{
	delete package;
}

ObjectFromFactory *PackageFactoryDll::GetObject(int classId) const
{
	return package->GetObject(classId);
}

LocalFactoryDll::LocalFactoryDll(DllObjectFactory::getObject_t getObject) :
	SUPER(), getObject(getObject)
{
}

LocalFactoryDll::~LocalFactoryDll()
{
}

ObjectFromFactory *LocalFactoryDll::GetObject(int classId) const
{
	if (classId < 0 ||
		classId > static_cast<int>(std::numeric_limits<MR_UInt16>::max()))
	{
		HR_LOG(warning) << "Out-of-range classId: " << classId;
		return nullptr;
	}
	else {
		return getObject(static_cast<MR_UInt16>(classId));
	}
}

}  // namespace Util
}  // namespace HoverRace
