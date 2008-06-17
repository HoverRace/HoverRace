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

#ifndef DLL_OBJECT_FACTORY_H
#define DLL_OBJECT_FACTORY_H

#include "MR_Types.h"

#ifdef MR_UTIL
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif

// Class decalration
class MR_ObjectFromFactoryId
{
   // Simple structure that hold the necessary information
   // tu uniquely identify a Factory Object
   public:
      MR_UInt16 mDllId;
      MR_UInt16 mClassId;

      void Serialize( CArchive& pArchive );

      MR_DllDeclare int operator == ( const MR_ObjectFromFactoryId& pId )const;
};


class MR_DllDeclare MR_ObjectFromFactory: public CObject
{   
   // Base class for object created with a Dll Factory

   private:
      
      MR_ObjectFromFactoryId mId;

   public:

      // Construction and destruction
      MR_ObjectFromFactory( const MR_ObjectFromFactoryId& pId );
      virtual ~MR_ObjectFromFactory();


      const MR_ObjectFromFactoryId& GetTypeId()const;

      // Serialisation functions
      //
      // Warning this module do not support multiple references to objects
      // or looped structures
      //
      // You can avoid this problem by calling CArchive::MapObject after aving call
      // SerializePtr. Then use CArchive operator << or >> CObject* the next time
      // you will have to serialize the object.(To use this technique, the first time 
      // you serialize the object, you must know that the object hav not been serialize yet)
      // 
      static  void SerializePtr( CArchive& pArchive, MR_ObjectFromFactory*& pPtr );
      virtual void Serialize( CArchive& pArchive );

};

namespace MR_DllObjectFactory
{
   MR_DllDeclare void Init();                      // Must be called at the begining of the program
   MR_DllDeclare void Clean( BOOL pOnlyDynamic );  // Must be called at the end of the program
                                                   // Can also be called to remove unused DLL

   // Low level function
   MR_DllDeclare BOOL OpenDll( MR_UInt16 pDllId );                   // Usually don't need to be called
   MR_DllDeclare void IncrementReferenceCount( MR_UInt16 pDllId );   // Usually don't need to be called
   MR_DllDeclare void DecrementReferenceCount( MR_UInt16 pDllId );   // Usually don't need to be called

   // Slow interrocation functions
   MR_DllDeclare MR_UInt16 GetObjectTypeCount( MR_UInt16 pDllId );
   MR_DllDeclare CString   GetObjectFamily   ( const MR_ObjectFromFactoryId& pId );
   MR_DllDeclare CString   GetObjectDescription( const MR_ObjectFromFactoryId& pId );
                                                 


   // Fast Object Creation function
   MR_DllDeclare MR_ObjectFromFactory* CreateObject( const MR_ObjectFromFactoryId& pId );


   // Local Dll 
   MR_DllDeclare void RegisterLocalDll( MR_UInt16 pDLLId, MR_ObjectFromFactory* (*pFunc)(MR_UInt16) );

};




#undef MR_DllDeclare

#endif
