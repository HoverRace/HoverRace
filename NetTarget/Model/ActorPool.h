// ActorPool.h
//

#ifndef ACTOR_POOL_H
#define ACTOR_POOL_H

#include "MazeElement.h"

#ifdef MR_MODEL
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif

// Class declaration
class   MR_FreeElementHandleClass;
typedef MR_FreeElementHandleClass* MR_FreeElementHandle;

// Class declaration
class MR_DllDeclare MR_ActorPool
{
   protected: 
      enum { eNonClassified  = -1 };

      class FreeElement
      {
         public:
            FreeElement**   mPrevLink;
            FreeElement*    mNext;
            MR_FreeElement* mElement;

            MR_DllDeclare   FreeElement();
            MR_DllDeclare   ~FreeElement();

            void Unlink();
            void LinkTo( FreeElement**  pPrevLink );

            void Serialize( CArchive& pArchive ); // See the comment in the implementation
      };

      int mNbRoom;
      FreeElement*  mNonClassifiedList;
      FreeElement** mClassifiedByRoomList;

   public:
      MR_ActorPool( int NbRoom);
      ~MR_ActorPool();

      void Serialize( CArchive& pArchive ); // See the comment in the implementation

      MR_FreeElementHandle          GetFirstFreeElement( int pRoom )const;
      static MR_FreeElementHandle   GetNextFreeElement( MR_FreeElementHandle pHandle );
      static MR_FreeElement*        GetFreeElement( MR_FreeElementHandle     pHandle );

      void                          MoveElement(   MR_FreeElementHandle pHandle, int pNewRoom );  // -1 mean non classified
      MR_FreeElementHandle          InsertElement( MR_FreeElement* pElement, int pNewRoom );      // -1 mean non classified
      static void                   DeleteElement( MR_FreeElementHandle pHandle );
};

#undef MR_DllDeclare
      
#endif
