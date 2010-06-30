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

#include "StdAfx.h"

#include <map>

#include "../ObjFac1/ObjFac1.h"
#include "../Parcel/ObjStream.h"

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
		BOOL mDynamic;
		int mRefCount;

		virtual ObjectFromFactory* GetObject(int classId) const = 0;

		// Initialisation
		FactoryDll();
		virtual ~FactoryDll();

};

// Using ObjFac1 (read from package file).
class PackageFactoryDll : public FactoryDll
{
	typedef FactoryDll SUPER;

	public:
		PackageFactoryDll();
		virtual ~PackageFactoryDll();

		virtual ObjectFromFactory* GetObject(int classId) const;

	private:
		ObjFac1::ObjFac1 *package;
};

// Using function pointer callback.
class LocalFactoryDll : public FactoryDll
{
	typedef FactoryDll SUPER;

	private:
		LocalFactoryDll() {}
	public:
		LocalFactoryDll(DllObjectFactory::getObject_t getObject);
		virtual ~LocalFactoryDll();

		virtual ObjectFromFactory* GetObject(int classId) const;

	private:
		DllObjectFactory::getObject_t getObject;
};

// Local functions declarations
static FactoryDll *GetDll(MR_UInt16 pDllId, BOOL pTrowOnError);

// Module variables
//TODO: Use a std::map instead.
typedef std::map<int,FactoryDll*> gsDllList_t;
static gsDllList_t gsDllList;

// Module functions
void DllObjectFactory::Init()
{
	// Nothing to do
}

void DllObjectFactory::Clean(BOOL pOnlyDynamic)
{
	for (gsDllList_t::iterator iter = gsDllList.begin(); iter != gsDllList.end(); ) {
		int dllId = iter->first;
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

/*
BOOL DllObjectFactory::OpenDll(MR_UInt16 pDllId)
{
	return (GetDll(pDllId, FALSE) != NULL);
}
*/

void DllObjectFactory::RegisterLocalDll(int pDllId, getObject_t pFunc)
{

	FactoryDll *lDllPtr;

	ASSERT(pDllId != 0);						  // Number 0 is reserved for NULL entry
	ASSERT(pDllId != 1);						  // Number 1 is reserved for ObjFac1
	ASSERT(pFunc != NULL);

	lDllPtr = new LocalFactoryDll(pFunc);

	bool inserted = gsDllList.insert(gsDllList_t::value_type(pDllId, lDllPtr)).second;
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
 * Get a handle to the factory DLL.  The option of choosing which DLL has been deprecated.
 */
FactoryDll *GetDll(MR_UInt16 pDllId, BOOL pThrowOnError)
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
ObjectFromFactory::ObjectFromFactory(const ObjectFromFactoryId & pId)
{
	mId = pId;

	// Increment this object dll reference count
	// This will prevent the Dll from being discarted
	DllObjectFactory::IncrementReferenceCount(mId.mDllId);

}

ObjectFromFactory::~ObjectFromFactory()
{

	// Decrement this object dll reference count
	// This will allow the dll to be freed if not needed anymore
	DllObjectFactory::DecrementReferenceCount(mId.mDllId);

}

const ObjectFromFactoryId &ObjectFromFactory::GetTypeId() const
{
	return mId;
}

void ObjectFromFactory::SerializePtr(ObjStream & pArchive, ObjectFromFactory * &pPtr)
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

void ObjectFromFactory::Serialize(ObjStream &pArchive)
{
	//CObject::Serialize(pArchive);

	// Notting to serialize at that point
	// Object type should be already initialize if Loading

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

int ObjectFromFactoryId::operator ==(const ObjectFromFactoryId & pId) const
{
	return ((mDllId == pId.mDllId) && (mClassId == pId.mClassId));
}


// class FactoryDll methods 
FactoryDll::FactoryDll() :
	mDynamic(FALSE), mRefCount(0)
{
}

FactoryDll::~FactoryDll()
{
	ASSERT(mRefCount == 0);
}

PackageFactoryDll::PackageFactoryDll() :
	SUPER()
{
	mDynamic = TRUE;
	package = new ObjFac1::ObjFac1();
}

PackageFactoryDll::~PackageFactoryDll()
{
	delete package;
}

ObjectFromFactory* PackageFactoryDll::GetObject(int classId) const
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

ObjectFromFactory* LocalFactoryDll::GetObject(int classId) const
{
	return getObject(classId);
}

}  // namespace Util
}  // namespace HoverRace
