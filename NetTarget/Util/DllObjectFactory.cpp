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

#include "stdafx.h"

#include "DllObjectFactory.h"

#define new DEBUG_NEW

// DllObjectFactory file name prefix
static const char gsFilePrefix[] = ".\\ObjFac";

// Local class declaration
class MR_FactoryDll
{
   // This class is used to kepp information about a loaded dll
   public:
      BOOL       mDynamic;
      int        mRefCount;
      HMODULE    mHandle;

      void                  (*mInitModule)( HMODULE pModule );
      void                  (*mCleanModule)();
      MR_UInt16             (*mGetObjectTypeCount)();
      CString               (*mGetObjectFamily)( MR_UInt16 pClassId );
      CString               (*mGetObjectDescription)( MR_UInt16 pClassId );
      MR_ObjectFromFactory* (*mGetObject)( MR_UInt16 pClassId );


      // Initialisation
      MR_FactoryDll();
      ~MR_FactoryDll();

      BOOL Open( MR_UInt16 pDllId ); // Must be called only once



};

// Local functions declarations
static MR_FactoryDll* GetDll( MR_UInt16 pDllId, BOOL pTrowOnError );


// Module variables
static CMap< MR_UInt16, MR_UInt16, MR_FactoryDll*, MR_FactoryDll* > gsDllList;
      
// Module functions
void MR_DllObjectFactory::Init()
{
   // Notting to do
}

void MR_DllObjectFactory::Clean( BOOL pOnlyDynamic )
{
   POSITION  lNextPos = gsDllList.GetStartPosition();


   while( lNextPos != NULL )
   {
      MR_UInt16         lDllId;
      MR_FactoryDll    *lDllPtr;

      gsDllList.GetNextAssoc( lNextPos, lDllId, lDllPtr );
  
      if( (lDllPtr->mRefCount <= 0)&&(lDllPtr->mDynamic || !pOnlyDynamic) )
      {
         // Remove the entry from the dictionnary
         gsDllList.RemoveKey( lDllId );

         // Free the dll
         delete lDllPtr;
      }
   }
}


BOOL MR_DllObjectFactory::OpenDll( MR_UInt16 pDllId )
{
   return( GetDll( pDllId, FALSE ) != NULL );
}

void MR_DllObjectFactory::RegisterLocalDll( MR_UInt16 pDllId, MR_ObjectFromFactory* (*pFunc)(MR_UInt16) )
{

   MR_FactoryDll* lDllPtr;

   ASSERT( pDllId != 0 ); // Number 0 is reserved for NULL entry
   ASSERT( pFunc != NULL );

   // Verify if the entry do not already exist
   ASSERT( !gsDllList.Lookup( pDllId, lDllPtr ) );

   lDllPtr = new MR_FactoryDll;

   lDllPtr->mGetObject = pFunc;

   gsDllList.SetAt( pDllId, lDllPtr );

      
}


void MR_DllObjectFactory::IncrementReferenceCount( MR_UInt16 pDllId )
{
   MR_FactoryDll* lDllPtr;

   if( gsDllList.Lookup( pDllId, lDllPtr ) )
   {
      lDllPtr->mRefCount++;
   }
   else
   {
      ASSERT( FALSE ); // Dll not loaded
   }

}

void MR_DllObjectFactory::DecrementReferenceCount( MR_UInt16 pDllId )
{
   MR_FactoryDll* lDllPtr;

   if( gsDllList.Lookup( pDllId, lDllPtr ) )
   {
      lDllPtr->mRefCount--;
   }
   else
   {
      ASSERT( FALSE ); // Dll discarted
   }
}

MR_UInt16 MR_DllObjectFactory::GetObjectTypeCount( MR_UInt16 pDllId )
{
   MR_UInt16 lReturnValue = 0;

   MR_FactoryDll* lDllPtr = GetDll( pDllId, TRUE );

   if( lDllPtr->mGetObjectTypeCount != NULL );
   {
      lReturnValue = lDllPtr->mGetObjectTypeCount(); 
   }   

   return lReturnValue;

}

CString   MR_DllObjectFactory::GetObjectFamily( const MR_ObjectFromFactoryId& pId )
{
   CString lReturnValue;

   MR_FactoryDll* lDllPtr = GetDll( pId.mDllId, TRUE );

   if( lDllPtr->mGetObjectFamily != NULL );
   {
      lReturnValue = lDllPtr->mGetObjectFamily( pId.mClassId ); 
   }  

   return lReturnValue;
}

CString   MR_DllObjectFactory::GetObjectDescription( const MR_ObjectFromFactoryId& pId )
{
   CString lReturnValue;

   MR_FactoryDll* lDllPtr = GetDll( pId.mDllId, TRUE );

   if( lDllPtr->mGetObjectDescription != NULL );
   {
      lReturnValue = lDllPtr->mGetObjectDescription( pId.mClassId ); 
   }   

   return lReturnValue;
}

MR_ObjectFromFactory* MR_DllObjectFactory::CreateObject( const MR_ObjectFromFactoryId& pId )
{
   MR_ObjectFromFactory* lReturnValue;


   MR_FactoryDll* lDllPtr = GetDll( pId.mDllId, TRUE );

   ASSERT( lDllPtr->mGetObject != NULL );
   
   lReturnValue = lDllPtr->mGetObject( pId.mClassId ); 
     

   return lReturnValue;
}



MR_FactoryDll* GetDll( MR_UInt16 pDllId, BOOL pTrowOnError )
{
   MR_FactoryDll* lDllPtr;

   ASSERT( pDllId != 0 ); // Number 0 is reserved for NULL entry

   // Verify if the entry do not already exist
   if( !gsDllList.Lookup( pDllId, lDllPtr ) )
   {
      lDllPtr = new MR_FactoryDll;
      
      if( !lDllPtr->Open( pDllId ) )
      {
         delete lDllPtr;

         if( pTrowOnError )
         {
            ASSERT( FALSE ); // Unable to open the DLL
            AfxThrowNotSupportedException();
         }
      }
      else
      {
         gsDllList.SetAt( pDllId, lDllPtr );
      }
   }

   return lDllPtr;
}



// MR_ObjectFromFactory methods
MR_ObjectFromFactory::MR_ObjectFromFactory( const MR_ObjectFromFactoryId& pId )
{
   mId   = pId;

   // Increment this object dll reference count
   // This will prevent the Dll from being discarted
   MR_DllObjectFactory::IncrementReferenceCount( mId.mDllId );

}

MR_ObjectFromFactory::~MR_ObjectFromFactory()
{
   
   // Decrement this object dll reference count
   // This will allow the dll to be freed if not needed anymore
   MR_DllObjectFactory::DecrementReferenceCount( mId.mDllId );

}


const MR_ObjectFromFactoryId& MR_ObjectFromFactory::GetTypeId()const
{
   return mId;
}

void MR_ObjectFromFactory::SerializePtr( CArchive& pArchive, MR_ObjectFromFactory*& pPtr )
{
   MR_ObjectFromFactoryId lId = {0,0};

   if( pArchive.IsStoring() )
   {
      if( pPtr != NULL )
      {
         lId   = pPtr->mId;
      }
      
      lId.Serialize( pArchive );               

      if( pPtr != NULL )
      {
         pPtr->Serialize( pArchive );
      }
   }
   else
   {
      lId.Serialize( pArchive );

      if( lId.mDllId == 0 )
      {
         pPtr = NULL;
      }
      else
      {
         pPtr = MR_DllObjectFactory::CreateObject( lId );
         pPtr->Serialize( pArchive );
      }
   }
}

void MR_ObjectFromFactory::Serialize( CArchive& pArchive )
{
   CObject::Serialize( pArchive );

   // Notting to serialize at that point
   // Object type should be already initialize if Loading
   
}

// MR_ObjectFromFactoryId
void MR_ObjectFromFactoryId::Serialize( CArchive& pArchive )
{
   if( pArchive.IsStoring() )
   {
      pArchive << mDllId
               << mClassId;

   }
   else
   {
      pArchive >> mDllId
               >> mClassId;
   }
}

int MR_ObjectFromFactoryId::operator ==( const MR_ObjectFromFactoryId& pId )const
{
   return( (mDllId == pId.mDllId)&&(mClassId==pId.mClassId));
}


// class MR_FactoryDll methods
      
MR_FactoryDll::MR_FactoryDll()
{
   mDynamic  = FALSE;
   mRefCount = 0;
   mHandle   = NULL;

   mInitModule           = NULL;
   mCleanModule          = NULL;
   mGetObjectTypeCount   = NULL;
   mGetObjectFamily      = NULL;
   mGetObjectDescription = NULL;
   mGetObject            = NULL;


}

MR_FactoryDll::~MR_FactoryDll()
{
   ASSERT( mRefCount == 0 );

   // Release the Dll
   if( mHandle != NULL )
   {
      if( mCleanModule != NULL )
      {
         mCleanModule();
      }
      FreeLibrary( mHandle );
   }
}

BOOL MR_FactoryDll::Open( MR_UInt16 pDllId )
{
   ASSERT( mHandle == NULL );

   char lNameBuffer[ 40 ];

   sprintf( lNameBuffer, "%s%u.dll", gsFilePrefix, pDllId );

   mDynamic = TRUE;

   mHandle = LoadLibrary( lNameBuffer );

   if(mHandle != NULL )
   {
      mInitModule           = (void (*)(HMODULE) )
                              GetProcAddress( mHandle,
                                              "MR_InitModule" );
      mCleanModule          = (void (*)() )
                              GetProcAddress( mHandle,
                                              "MR_CleanModule" );
      mGetObjectTypeCount   = (MR_UInt16 (*)() )
                              GetProcAddress( mHandle,
                                              "MR_GetObjectTypeCount" );
      mGetObjectFamily      = (CString (*)( MR_UInt16 ))
                              GetProcAddress( mHandle,
                                              "MR_GetObjectFamily" );
      mGetObjectDescription = (CString (*)( MR_UInt16 ))
                              GetProcAddress( mHandle,
                                              "MR_GetObjectDescription" );
      mGetObject            = ( MR_ObjectFromFactory* (*)( MR_UInt16 ))
                              GetProcAddress( mHandle,
                                              "MR_GetObject" );

      if( mInitModule != NULL )
      {
         mInitModule( mHandle );
      }

   }

   return( mHandle != NULL );
}

