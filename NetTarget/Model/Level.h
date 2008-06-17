// Level.h
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

#ifndef LEVEL_H
#define LEVEL_H

#include "MazeElement.h"
#include "ShapeCollisions.h"
#include "../Util/FastArray.h"

#ifdef MR_MODEL
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif

// Defines
#define MR_NB_MAX_PLAYER    32
#define MR_NB_PERNET_ACTORS 64

// Class declaration
class   MR_FreeElementHandleClass;
typedef MR_FreeElementHandleClass* MR_FreeElementHandle;


class MR_SectionId
{
   public:
      enum eSectionType{ eRoom, eFeature };

      eSectionType mType;
      int          mId;

      void Serialize(  CArchive& pArchive );
};

class MR_DllDeclare  MR_Level
{

   // This class will be derived for construction purpose by the MazeCompiler
   // that is why it is only protected, not private
   //
   // This class will also be overrided by the simulator      
   // Simulator related methods and members will than have to be       
   // moved in the MR_SimulatorLevel class

   public:
      enum { eNonClassified  = -1, eMustBeDeleted = -2 };

   protected: 

      // Class pre-declaration
      class Section;

      // Helper structure
      class MR_DllDeclare SectionShape:public MR_PolygonShape
      {
         private:
            Section* mSection;

         public:
            SectionShape( Section *pSection );

            MR_Int32  XMin()const;
            MR_Int32  XMax()const;
            MR_Int32  YMin()const;
            MR_Int32  YMax()const;
            MR_Int32  ZMin()const;
            MR_Int32  ZMax()const;

            int       VertexCount()const;
            MR_Int32  XCenter()const;
            MR_Int32  YCenter()const;

            MR_Int32  X( int pIndex )const;
            MR_Int32  Y( int pIndex )const;
            MR_Int32  SideLen( int pIndex )const;
            
      };


      // Private structures
      
      class MR_DllDeclare Section
      {
         public:

            int  mNbVertex; // Number of vertex that compose the section

            // Geometry
            MR_Int32         mFloorLevel;
            MR_Int32         mCeilingLevel;
            MR_2DCoordinate* mVertexList;
            MR_Int32*        mWallLen;

            // Dounding box geometry
            MR_2DCoordinate mMin;   
            MR_2DCoordinate mMax;

            // Wall textures
            MR_SurfaceElement** mWallTexture;
            MR_SurfaceElement*  mFloorTexture;
            MR_SurfaceElement*  mCeilingTexture;


            // Methods
            Section();
            ~Section();

            void SerializeStructure( CArchive& pArchive );
            void SerializeSurfacesLogicState( CArchive& pArchive );

      };


      class Feature: public Section
      {
         public:
            // Connectivity
            int  mParentSectionIndex;

            void SerializeStructure(  CArchive& pArchive );
      };

      class MR_DllDeclare Room: public Section
      {
         public:
            // Local structures

            class AudibleRoom
            {
            public:

               // This class will have be modified if we want to take in account the
               // fact that a closed door do not let pass the sound
               int   mSectionSource;
               int   mNbVertexSources;  // Number of connection from where the sound is comming
               int*  mVertexList;       // List of the connections from where the sound is comming
               BYTE* mSoundCoefficient; // Attenuation factor of the sound

               AudibleRoom();
               ~AudibleRoom();

               void Serialize(  CArchive& pArchive );

            };


            // Connectivity
            int  mNbChild;
            int* mChildList;          // Index of the child sections

            int* mNeighborList;       // One entry per polygon side
                                      // Sides with no Neighbor are mark with -1
                                      // this is a Room specific member


            // Pre-computed inter-room effects
            int    mNbVisibleRoom;    // Size of mVisibleRoomList
            int*   mVisibleRoomList;  // List of the room that are visible from the current room
                                      // This attribute apply only to Sections without parent

            int            mNbVisibleSurface;
            MR_SectionId* mVisibleFloorList;   // Ordered floor list
            MR_SectionId* mVisibleCeilingList; // Ordered ceiling list


            int           mNbAudibleRoom;    // Theses members are only used by the server
            AudibleRoom*  mAudibleRoomList;  // List of the room from where a sound can be heard from the current room
                                             
            
            // Methods
            Room();
            ~Room();
            
            void SerializeStructure(  CArchive& pArchive );

      };

      class FreeElement
      {
         public:
            FreeElement**   mPrevLink;
            FreeElement*    mNext;
            MR_FreeElement* mElement;

            FreeElement();
            ~FreeElement();

            void Unlink();
            void LinkTo( FreeElement**  pPrevLink );

            static void SerializeList( CArchive& pArchive, FreeElement** pListHead );
      };


      // Private Data
      // Level structure      
      BOOL             mAllowRendering;

      int              mNbRoom;    // Number of room in the level
      Room*            mRoomList;

      int              mNbFeature;   // Number of features in the level
      Feature*         mFeatureList; 
   
      // Main character starting position and references
      int                   mNbPlayer;
      int                   mPlayerTeam[MR_NB_MAX_PLAYER ];
      int                   mStartingRoom[ MR_NB_MAX_PLAYER ];
      MR_3DCoordinate       mStartingPosition[ MR_NB_MAX_PLAYER ];
      MR_Angle              mStartingOrientation[ MR_NB_MAX_PLAYER ];

      // FreeElements
      FreeElement*  mFreeElementNonClassifiedList;
      FreeElement** mFreeElementClassifiedByRoomList;

      int           mNbPermNetActor;
      FreeElement*  mPermNetActor[ MR_NB_PERNET_ACTORS ];

      // PermActor moving cache (Big patch since there is a smll bug in the design)
      int           mPermActorCacheCount;
      int           mPermActorIndexCache[ MR_NB_PERNET_ACTORS ];
      int           mPermActorNewRoomCache[ MR_NB_PERNET_ACTORS ];


      // Broadcast hook
      void (*mElementCreationBroadcastHook)( MR_FreeElement* pElement, int pRoom, void* pHookData );
      void (*mPermElementStateBroadcastHook)( MR_FreeElement* pElement, int pRoom, int pPermId, void* pHookData );
      void* mBroadcastHookData;

      // Helper functions
      int  GetRealRoomRecursive( const MR_2DCoordinate& pPosition, int pOriginalSection, int = -1 )const;

   public:
      MR_Level( BOOL pAllowRendering = FALSE );
      ~MR_Level();

      // Network stuff
      void SetBroadcastHook( void (*pCreationHook)(MR_FreeElement*, int, void* ),
                             void (*pStateHook)   (MR_FreeElement*, int, int, void* ),
                             void* pHookData );


      // Serialisation functions
      void Serialize( CArchive& pArchive );


      // Strucre Interrocation functions
      int GetRoomCount()const;

      // Starting pos related stuff
      int                    GetPlayerCount()const;
      int                    GetPlayerTeam( int pPlayerId )const;
      int                    GetStartingRoom(  int pPlayerId )const;
      const MR_3DCoordinate& GetStartingPos(  int pPlayerId )const;
      MR_Angle               GetStartingOrientation( int pPlayerId )const;

      // Structural information
      MR_PolygonShape*       GetRoomShape(       int pRoomId )const;
      MR_Int32               GetRoomWallLen(     int pRoomId, int pVertex )const;
      MR_Int32               GetRoomBottomLevel( int pRoomId )const;
      MR_Int32               GetRoomTopLevel(    int pRoomId )const;
      const MR_2DCoordinate& GetRoomVertex(      int pRoomId, int pVertex )const;
      int                    GetRoomVertexCount( int pRoomId )const;

      MR_PolygonShape*       GetFeatureShape(       int pFeatureId )const;
      MR_Int32               GetFeatureWallLen(     int pFeatureId, int pVertex )const;
      MR_Int32               GetFeatureBottomLevel( int pFeatureId )const;
      MR_Int32               GetFeatureTopLevel(    int pFeatureId )const;
      const MR_2DCoordinate& GetFeatureVertex(      int pFeatureId, int pVertex )const;
      int                    GetFeatureVertexCount( int pFeatureId )const;

      const int*          GetVisibleZones(       int pRoomId, int& pNbVisibleZones )const;
      int                 GetNbVisibleSurface(   int pRoomId )const;
      const MR_SectionId* GetVisibleFloorList(   int pRoomId )const;
      const MR_SectionId* GetVisibleCeilingList( int pRoomId )const;
      int                 GetNeighbor(           int pRoomId, int  pVertex )const;
      int                 GetParent(             int pFeatureId )const;
      int                 GetFeatureCount(       int pRoomId )const;
      int                 GetFeature(            int pRoomId, int pChildIndex )const;

      MR_SurfaceElement* GetRoomWallElement(     int pRoomId, int  pVertex )const;    
      MR_SurfaceElement* GetRoomBottomElement(   int pRoomId )const;    
      MR_SurfaceElement* GetRoomTopElement(      int pRoomId )const;    

      MR_SurfaceElement* GetFeatureWallElement(   int pFeatureId, int  pVertex )const;    
      MR_SurfaceElement* GetFeatureBottomElement( int pFeatureId )const;    
      MR_SurfaceElement* GetFeatureTopElement(    int pFeatureId )const;    

      // Free element content interrogation and manipulation
      MR_FreeElementHandle          GetFirstFreeElement( int pRoom )const;
      static MR_FreeElementHandle   GetNextFreeElement( MR_FreeElementHandle pHandle );
      static MR_FreeElement*        GetFreeElement( MR_FreeElementHandle     pHandle );
      MR_FreeElementHandle          GetPermanentElementHandle( int pElem )const;

      void                          MoveElement(   MR_FreeElementHandle pHandle, int pNewRoom );                          // -1 mean non classified
      MR_FreeElementHandle          InsertElement( MR_FreeElement* pElement, int pNewRoom, BOOL Broadcast = FALSE );      // -1 mean non classified
      static void                   DeleteElement( MR_FreeElementHandle pHandle );

      void                          SetPermElementPos( int pPermElement, int pRoom, const MR_3DCoordinate& pNewPos );      // Set and broadcast the newporition
      void                          FlushPermElementPosCache();

      // Element movement functions
      int  FindRoomForPoint( const MR_2DCoordinate& pPosition, int pStartingRoom )const;

      void GetRoomContact(    int                      pRoom,
                              const MR_ShapeInterface* pShape,
                              MR_RoomContactSpec&      pAnswer );

      BOOL GetRoomWallContactOrientation( int                      pRoom,
                                          int                      pWall,
                                          const MR_ShapeInterface* pShape,
                                          MR_Angle&                pAnswer );


      BOOL GetFeatureContact( int                      pFeature,
                              const MR_ShapeInterface* pShape,
                              MR_ContactSpec&          pAnswer    );


      BOOL GetFeatureContactOrientation( int                      pFeature,
                                         const MR_ShapeInterface* pShape,
                                         MR_Angle&                pAnswer   );




};

#undef MR_DllDeclare
      
#endif
