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

#pragma once

#include "MazeElement.h"
#include "ShapeCollisions.h"
#include "../Util/FastArray.h"

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

// Defines
#define MR_NB_MAX_PLAYER    32
#define MR_NB_PERNET_ACTORS 512

// for game options (also found in TrackSelect.h)
#define OPT_ALLOW_WEAPONS	0x40
#define OPT_ALLOW_MINES		0x20
#define OPT_ALLOW_CANS		0x10
#define OPT_ALLOW_BASIC		0x08
#define OPT_ALLOW_BI		0x02
#define OPT_ALLOW_CX		0x04
#define OPT_ALLOW_EON		0x01

// Class declaration
namespace HoverRace {
	namespace Parcel {
		class ObjStream;
	}
}
class MR_FreeElementHandleClass;
typedef MR_FreeElementHandleClass *MR_FreeElementHandle;

namespace HoverRace {
namespace Model {

class SectionId
{
	public:
		enum eSectionType { eRoom, eFeature };

		eSectionType mType;
		int mId;

		void Serialize(Parcel::ObjStream &pArchive);
};

class MR_DllDeclare Level
{

	// This class will be derived for construction purpose by the MazeCompiler
	// that is why it is only protected, not private
	//
	// This class will also be overrided by the simulator
	// Simulator related methods and members will than have to be
	// moved in the MR_SimulatorLevel class

	public:
		enum { eNonClassified = -1, eMustBeDeleted = -2 };

	protected:

		// Class pre-declaration
		class Section;

		// Helper structure
		class MR_DllDeclare SectionShape : public Model::PolygonShape
		{
			private:
				Section * mSection;

			public:
				SectionShape(Section * pSection);

				MR_Int32 XMin() const;
				MR_Int32 XMax() const;
				MR_Int32 YMin() const;
				MR_Int32 YMax() const;
				MR_Int32 ZMin() const;
				MR_Int32 ZMax() const;

				int VertexCount() const;
				MR_Int32 XCenter() const;
				MR_Int32 YCenter() const;

				MR_Int32 X(int pIndex) const;
				MR_Int32 Y(int pIndex) const;
				MR_Int32 SideLen(int pIndex) const;

		};

		// Private structures

		class MR_DllDeclare Section
		{
			public:

				int mNbVertex;					  // Number of vertex that compose the section

				// Geometry
				MR_Int32 mFloorLevel;
				MR_Int32 mCeilingLevel;
				MR_2DCoordinate *mVertexList;
				MR_Int32 *mWallLen;

				// Dounding box geometry
				MR_2DCoordinate mMin;
				MR_2DCoordinate mMax;

				// Wall textures
				SurfaceElement **mWallTexture;
				SurfaceElement *mFloorTexture;
				SurfaceElement *mCeilingTexture;

				// Methods
				Section();
				~Section();

				void SerializeStructure(Parcel::ObjStream &pArchive);
				void SerializeSurfacesLogicState(Parcel::ObjStream &pArchive);

		};

		class Feature : public Section
		{
			public:
				// Connectivity
				int mParentSectionIndex;

				void SerializeStructure(Parcel::ObjStream &pArchive);
		};

		class MR_DllDeclare Room : public Section
		{
			public:
				// Local structures

				class AudibleRoom
				{
					public:

						// This class will have be modified if we want to take in account the
						// fact that a closed door do not let pass the sound
						int mSectionSource;
						int mNbVertexSources;	  // Number of connection from where the sound is comming
						int *mVertexList;		  // List of the connections from where the sound is comming
						BYTE *mSoundCoefficient;  // Attenuation factor of the sound

						AudibleRoom();
						~AudibleRoom();

						void Serialize(Parcel::ObjStream &pArchive);

				};

				// Connectivity
				int mNbChild;
				int *mChildList;				  // Index of the child sections

				int *mNeighborList;				  // One entry per polygon side
				// Sides with no Neighbor are mark with -1
				// this is a Room specific member

				// Pre-computed inter-room effects
				int mNbVisibleRoom;				  // Size of mVisibleRoomList
				int *mVisibleRoomList;			  // List of the room that are visible from the current room
				// This attribute apply only to Sections without parent

				int mNbVisibleSurface;
				SectionId *mVisibleFloorList;  // Ordered floor list
				SectionId *mVisibleCeilingList;// Ordered ceiling list

				int mNbAudibleRoom;				  // Theses members are only used by the server
				AudibleRoom *mAudibleRoomList;	  // List of the room from where a sound can be heard from the current room

				// Methods
				Room();
				~Room();

				void SerializeStructure(Parcel::ObjStream &pArchive);

		};

		class FreeElementList
		{
			public:
				FreeElementList ** mPrevLink;
				FreeElementList *mNext;
				FreeElement *mElement;

				FreeElementList();
				~FreeElementList();

				void Unlink();
				void LinkTo(FreeElementList ** pPrevLink);

				static void SerializeList(Parcel::ObjStream &pArchive, FreeElementList **pListHead);
		};

		// Private Data
		// Level structure
		BOOL mAllowRendering;

		int mNbRoom;							  // Number of room in the level
		Room *mRoomList;

		int mNbFeature;							  // Number of features in the level
		Feature *mFeatureList;

		char mGameOpts;

		// Main character starting position and references
		int mNbPlayer;
		int mPlayerTeam[MR_NB_MAX_PLAYER];
		int mStartingRoom[MR_NB_MAX_PLAYER];
		MR_3DCoordinate mStartingPosition[MR_NB_MAX_PLAYER];
		MR_Angle mStartingOrientation[MR_NB_MAX_PLAYER];

		// FreeElements
		FreeElementList *mFreeElementNonClassifiedList;
		FreeElementList **mFreeElementClassifiedByRoomList;

		int mNbPermNetActor;
		FreeElementList *mPermNetActor[MR_NB_PERNET_ACTORS];

		// PermActor moving cache (Big patch since there is a smll bug in the design)
		int mPermActorCacheCount;
		int mPermActorIndexCache[MR_NB_PERNET_ACTORS];
		int mPermActorNewRoomCache[MR_NB_PERNET_ACTORS];

		// Broadcast hook
		void (*mElementCreationBroadcastHook) (FreeElement * pElement, int pRoom, void *pHookData);
		void (*mPermElementStateBroadcastHook) (FreeElement * pElement, int pRoom, int pPermId, void *pHookData);
		void *mBroadcastHookData;

		// Helper functions
		int GetRealRoomRecursive(const MR_2DCoordinate & pPosition, int pOriginalSection, int = -1) const;

	public:
		Level(BOOL pAllowRendering = FALSE, char pGameOpts = 1);
		~Level();

		// Network stuff
		void SetBroadcastHook(void (*pCreationHook) (FreeElement *, int, void *), void (*pStateHook) (FreeElement *, int, int, void *), void *pHookData);

		// Serialisation functions
		void Serialize(Parcel::ObjStream &pArchive);

		// Strucre Interrocation functions
		int GetRoomCount() const;

		// Starting pos related stuff
		int GetPlayerCount() const;
		int GetPlayerTeam(int pPlayerId) const;
		int GetStartingRoom(int pPlayerId) const;
		const MR_3DCoordinate & GetStartingPos(int pPlayerId) const;
		MR_Angle GetStartingOrientation(int pPlayerId) const;

		// Structural information
		PolygonShape *GetRoomShape(int pRoomId) const;
		MR_Int32 GetRoomWallLen(int pRoomId, int pVertex) const;
		MR_Int32 GetRoomBottomLevel(int pRoomId) const;
		MR_Int32 GetRoomTopLevel(int pRoomId) const;
		const MR_2DCoordinate & GetRoomVertex(int pRoomId, int pVertex) const;
		int GetRoomVertexCount(int pRoomId) const;

		PolygonShape *GetFeatureShape(int pFeatureId) const;
		MR_Int32 GetFeatureWallLen(int pFeatureId, int pVertex) const;
		MR_Int32 GetFeatureBottomLevel(int pFeatureId) const;
		MR_Int32 GetFeatureTopLevel(int pFeatureId) const;
		const MR_2DCoordinate & GetFeatureVertex(int pFeatureId, int pVertex) const;
		int GetFeatureVertexCount(int pFeatureId) const;

		const int *GetVisibleZones(int pRoomId, int &pNbVisibleZones) const;
		int GetNbVisibleSurface(int pRoomId) const;
		const SectionId *GetVisibleFloorList(int pRoomId) const;
		const SectionId *GetVisibleCeilingList(int pRoomId) const;
		int GetNeighbor(int pRoomId, int pVertex) const;
		int GetParent(int pFeatureId) const;
		int GetFeatureCount(int pRoomId) const;
		int GetFeature(int pRoomId, int pChildIndex) const;

		SurfaceElement *GetRoomWallElement(int pRoomId, int pVertex) const;
		SurfaceElement *GetRoomBottomElement(int pRoomId) const;
		SurfaceElement *GetRoomTopElement(int pRoomId) const;

		SurfaceElement *GetFeatureWallElement(int pFeatureId, int pVertex) const;
		SurfaceElement *GetFeatureBottomElement(int pFeatureId) const;
		SurfaceElement *GetFeatureTopElement(int pFeatureId) const;

		// Free element content interrogation and manipulation
		MR_FreeElementHandle GetFirstFreeElement(int pRoom) const;
		static MR_FreeElementHandle GetNextFreeElement(MR_FreeElementHandle pHandle);
		static FreeElement *GetFreeElement(MR_FreeElementHandle pHandle);
		MR_FreeElementHandle GetPermanentElementHandle(int pElem) const;

												  // -1 mean non classified
		void MoveElement(MR_FreeElementHandle pHandle, int pNewRoom);
												  // -1 mean non classified
		MR_FreeElementHandle InsertElement(FreeElement * pElement, int pNewRoom, BOOL Broadcast = FALSE);
		static void DeleteElement(MR_FreeElementHandle pHandle);

												  // Set and broadcast the newporition
		void SetPermElementPos(int pPermElement, int pRoom, const MR_3DCoordinate & pNewPos);
		void FlushPermElementPosCache();

		// Element movement functions
		int FindRoomForPoint(const MR_2DCoordinate & pPosition, int pStartingRoom) const;

		void GetRoomContact(int pRoom, const ShapeInterface * pShape, RoomContactSpec & pAnswer);

		BOOL GetRoomWallContactOrientation(int pRoom, int pWall, const ShapeInterface * pShape, MR_Angle & pAnswer);

		BOOL GetFeatureContact(int pFeature, const ShapeInterface * pShape, ContactSpec & pAnswer);

		BOOL GetFeatureContactOrientation(int pFeature, const ShapeInterface * pShape, MR_Angle & pAnswer);

};

}  // namespace Model
}  // namespace HoverRace

#undef MR_DllDeclare
