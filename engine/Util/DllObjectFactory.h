// DllObjectFactory.h
//
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

//
// This file constais fonctions for creating objects from
// two integer numbers. One of this number identify the
// dll containing the object to create and the other contain
// factory class
//
//

#pragma once

#include "../Parcel/ObjStream.h"

#include "MR_Types.h"

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
	namespace ObjFacTools {
		class ResourceLib;
	}
}

namespace HoverRace {
namespace Util {

/// Unique identifier for a Factory Object.
struct ObjectFromFactoryId
{
	MR_UInt16 mDllId;
	MR_UInt16 mClassId;

	void Serialize(Parcel::ObjStream &pArchive);

	bool operator==(const ObjectFromFactoryId &pId) const
	{
		return
			mDllId == pId.mDllId &&
			mClassId == pId.mClassId;
	}
};

class ObjectFromFactory;

namespace DllObjectFactory {

/// Must be called at the beginning of the program.
MR_DllDeclare void Init();

/// Must be called at the end of the program.
MR_DllDeclare void Clean();

/// Fast Object Creation function
MR_DllDeclare ObjectFromFactory *CreateObject(const ObjectFromFactoryId &pId);

MR_DllDeclare ObjFacTools::ResourceLib &GetResourceLib(MR_UInt16 dllId = 1);

}  // namespace DllObjectFactory

/// Base class for object created with a Dll Factory
class MR_DllDeclare ObjectFromFactory
{
private:
	ObjectFromFactoryId mId;

public:
	ObjectFromFactory(const ObjectFromFactoryId &pId) : mId(pId) { }
	virtual ~ObjectFromFactory() { }

	const ObjectFromFactoryId &GetTypeId() const { return mId; }

private:
	static void ThrowUnexpected(const ObjectFromFactoryId &oid);

public:
	// Serialisation functions
	//
	// Warning this module do not support multiple references to objects
	// or looped structures

	/**
	 * Serialize a shared pointer to a serializable object.
	 *
	 * When reading, the created object is checked against the expected type.
	 * If the created object is not the correct type, then the object pointer
	 * is set to @c nullptr.
	 *
	 * @tparam T The expected shared pointer type.
	 * @param archive The archive.
	 * @param [in,out] obj The object.
	 * @throw Parcel::ObjStreamExn The object being read from the stream is
	 *                             not the expected type.
	 */
	template<class T>
	static void SerializeShared(Parcel::ObjStream &archive,
		typename std::enable_if<
			std::is_base_of<ObjectFromFactory, T>::value,
			std::shared_ptr<T>>::type &obj)
	{
		ObjectFromFactoryId oid = { 0, 0 };

		if (archive.IsWriting()) {
			if (obj) {
				oid = obj->mId;
			}
			oid.Serialize(archive);
			if (obj) {
				obj->Serialize(archive);
			}
		}
		else {
			oid.Serialize(archive);

			if (oid.mDllId == 0) {
				obj.reset();
			}
			else {
				auto *newObj = DllObjectFactory::CreateObject(oid);

				T *dynObj = dynamic_cast<T*>(newObj);
				if (!dynObj) {
					obj.reset();
					delete newObj;
					// The ID is probably corrupt or unsupported, so we can't
					// rely on the rest of the stream (we don't know where the
					// next object starts!).
					ThrowUnexpected(oid);
				}
				obj.reset(dynObj);
				obj->Serialize(archive);
			}
		}
	}

	virtual void Serialize(Parcel::ObjStream &archive) { HR_UNUSED(archive); }
};

}  // namespace Util
}  // namespace HoverRace

#undef MR_DllDeclare
