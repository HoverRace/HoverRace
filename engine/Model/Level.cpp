
// Level.cpp
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

#include "../Parcel/ObjStream.h"
#include "Track.h"

#include "Level.h"

using namespace HoverRace::Util;
using HoverRace::Parcel::ObjStream;

namespace HoverRace {
namespace Model {

Level::Level(Track &track, BOOL pAllowRendering, char pGameOpts) :
	track(track)
{
	// Initialization of an empty level
	mAllowRendering = pAllowRendering;

	mNbRoom = 0;
	mRoomList = NULL;
	mNbFeature = 0;
	mFeatureList = NULL;
	mFreeElementNonClassifiedList = NULL;
	mFreeElementClassifiedByRoomList = NULL;
	mNbPlayer = 0;

	mGameOpts = pGameOpts;

	mElementCreationBroadcastHook = NULL;
	mPermElementStateBroadcastHook = NULL;
	// Initialize the players list

	mNbPermNetActor = 0;
	mPermActorCacheCount = 0;

}

Level::~Level()
{

	// Delete free elements
	while(mFreeElementNonClassifiedList != NULL) {
		delete mFreeElementNonClassifiedList;
	}

	for(int lCounter = 0; lCounter < mNbRoom; lCounter++) {
		while(mFreeElementClassifiedByRoomList[lCounter] != NULL) {
			delete mFreeElementClassifiedByRoomList[lCounter];
		}
	}

	delete[]mFreeElementClassifiedByRoomList;

	// Delete structure
	delete[]mRoomList;
	delete[]mFeatureList;

}

void Level::SetBroadcastHook(void (*pCreationHook) (FreeElement *, int, void *), void (*pStateHook) (FreeElement *, int, int, void *), void *pHookData)
{
	mElementCreationBroadcastHook = pCreationHook;
	mPermElementStateBroadcastHook = pStateHook;
	mBroadcastHookData = pHookData;
}

// Serialization
void Level::Serialize(ObjStream &pArchive)
{
	int lCounter;
	int lPlayerNo;

	// Serialize the starting position
	if(pArchive.IsWriting()) {
		pArchive << mNbPlayer;

		for(lPlayerNo = 0; lPlayerNo < mNbPlayer; lPlayerNo++) {
			pArchive << mPlayerTeam[lPlayerNo];
			pArchive << mStartingRoom[lPlayerNo];
			mStartingPosition[lPlayerNo].Serialize(pArchive);
			pArchive << mStartingOrientation[lPlayerNo];
		}
	}
	else {
		mNbPermNetActor = 0;

		pArchive >> mNbPlayer;

		for(lPlayerNo = 0; lPlayerNo < mNbPlayer; lPlayerNo++) {
			pArchive >> mPlayerTeam[lPlayerNo];
			pArchive >> mStartingRoom[lPlayerNo];
			mStartingPosition[lPlayerNo].Serialize(pArchive);
			pArchive >> mStartingOrientation[lPlayerNo];
		}
	}

	// Serialize the structure
	if(pArchive.IsWriting()) {
		pArchive << mNbRoom;
		pArchive << mNbFeature;
	}
	else {
		ASSERT(mRoomList == NULL);
		ASSERT(mFeatureList == NULL);

		pArchive >> mNbRoom;
		pArchive >> mNbFeature;

		mRoomList = new Room[mNbRoom];
		mFeatureList = new Feature[mNbFeature];
		mFreeElementClassifiedByRoomList = new FreeElementList *[mNbRoom];

		for(lCounter = 0; lCounter < mNbRoom; lCounter++) {
			mFreeElementClassifiedByRoomList[lCounter] = NULL;
		}
	}

	for(lCounter = 0; lCounter < mNbRoom; lCounter++) {
		mRoomList[lCounter].SerializeStructure(pArchive);
	}

	for(lCounter = 0; lCounter < mNbFeature; lCounter++) {
		mFeatureList[lCounter].SerializeStructure(pArchive);
	}

	// Serialise the actors

	FreeElementList::SerializeList(pArchive, &mFreeElementNonClassifiedList);

	for(lCounter = 0; lCounter < mNbRoom; lCounter++) {
		FreeElementList::SerializeList(pArchive, &mFreeElementClassifiedByRoomList[lCounter]);

		if(!pArchive.IsWriting()) {
			FreeElementList *lCurrentElem = mFreeElementClassifiedByRoomList[lCounter];

			while(lCurrentElem != NULL) {
				if(mNbPermNetActor < MR_NB_PERNET_ACTORS) {
					// offer the current number to the current actor
					if(lCurrentElem->mElement->AssignPermNumber(mNbPermNetActor)) {
						mPermNetActor[mNbPermNetActor] = lCurrentElem;
						mNbPermNetActor++;
					}
				}
				else {
					ASSERT(FALSE);
				}
				lCurrentElem = lCurrentElem->mNext;
			}
		}
	}

	// remove unwanted elements
	for(lCounter = 0; lCounter < mNbRoom; lCounter++) {
		if(!pArchive.IsWriting()) {
			FreeElementList *lCurrentElem = mFreeElementClassifiedByRoomList[lCounter];


			while(lCurrentElem != NULL) {
				FreeElementList *lNext = lCurrentElem->mNext;

				// check if the element is allowed
				if((lCurrentElem->mElement->GetTypeId().mClassId == 151 /* mine */) && !(mGameOpts & OPT_ALLOW_MINES)) {
					// move to unusable place
					lCurrentElem->LinkTo(&mFreeElementNonClassifiedList);
				} else if((lCurrentElem->mElement->GetTypeId().mClassId == 152 /* can */) && !(mGameOpts & OPT_ALLOW_CANS)) {
					lCurrentElem->LinkTo(&mFreeElementNonClassifiedList);
				}

				lCurrentElem = lNext;
			}
		}
	}
}

// Internal helper functions

int Level::GetRoomCount() const
{
	return mNbRoom;
}

int Level::GetPlayerCount() const
{
	return mNbPlayer;
}

int Level::GetPlayerTeam(int pPlayerId) const
{
	return mPlayerTeam[pPlayerId];
}

int Level::GetStartingRoom(int pPlayerId) const
{
	return mStartingRoom[pPlayerId];
}

const MR_3DCoordinate & Level::GetStartingPos(int pPlayerId) const
{
	return mStartingPosition[pPlayerId];
}

MR_Angle Level::GetStartingOrientation(int pPlayerId) const
{
	return mStartingOrientation[pPlayerId];
}

PolygonShape *Level::GetRoomShape(int pRoomId) const
{
	return new SectionShape(&mRoomList[pRoomId]);
}

PolygonShape *Level::GetFeatureShape(int pFeatureId) const
{
	return new SectionShape(&mFeatureList[pFeatureId]);
}

MR_Int32 Level::GetRoomWallLen(int pRoomId, int pVertex) const
{
	return mRoomList[pRoomId].mWallLen[pVertex];
}

MR_Int32 Level::GetFeatureWallLen(int pFeatureId, int pVertex) const
{
	return mFeatureList[pFeatureId].mWallLen[pVertex];
}

const MR_2DCoordinate & Level::GetRoomVertex(int pRoomId, int pVertex) const
{
	return mRoomList[pRoomId].mVertexList[pVertex];
}

const MR_2DCoordinate & Level::GetFeatureVertex(int pFeatureId, int pVertex) const
{
	return mFeatureList[pFeatureId].mVertexList[pVertex];
}

int Level::GetRoomVertexCount(int pRoomId) const
{
	return mRoomList[pRoomId].mNbVertex;
}

int Level::GetFeatureVertexCount(int pFeatureId) const
{
	return mFeatureList[pFeatureId].mNbVertex;
}

MR_Int32 Level::GetRoomBottomLevel(int pRoomId) const
{
	return mRoomList[pRoomId].mFloorLevel;
}

MR_Int32 Level::GetFeatureBottomLevel(int pFeatureId) const
{
	return mFeatureList[pFeatureId].mFloorLevel;
}

MR_Int32 Level::GetRoomTopLevel(int pRoomId) const
{
	return mRoomList[pRoomId].mCeilingLevel;
}

MR_Int32 Level::GetFeatureTopLevel(int pFeatureId) const
{
	return mFeatureList[pFeatureId].mCeilingLevel;
}

const int *Level::GetVisibleZones(int pRoomId, int &pNbVisibleZones) const
{

	pNbVisibleZones = mRoomList[pRoomId].mNbVisibleRoom;

	return mRoomList[pRoomId].mVisibleRoomList;
}

int Level::GetNbVisibleSurface(int pRoomId) const
{
	return mRoomList[pRoomId].mNbVisibleSurface;
}

const SectionId *Level::GetVisibleFloorList(int pRoomId) const
{
	return mRoomList[pRoomId].mVisibleFloorList;
}

const SectionId *Level::GetVisibleCeilingList(int pRoomId) const
{
	return mRoomList[pRoomId].mVisibleCeilingList;
}

int Level::GetNeighbor(int pRoomId, int pVertex) const
{
	return mRoomList[pRoomId].mNeighborList[pVertex];
}

int Level::GetParent(int pFeatureId) const
{
	return mFeatureList[pFeatureId].mParentSectionIndex;
}

int Level::GetFeatureCount(int pRoomId) const
{
	return mRoomList[pRoomId].mNbChild;
}

int Level::GetFeature(int pRoomId, int pChildIndex) const
{
	return mRoomList[pRoomId].mChildList[pChildIndex];
}

SurfaceElement *Level::GetRoomWallElement(int pRoomId, size_t pVertex) const
{
	return mRoomList[pRoomId].mWallTexture[pVertex].get();
}

SurfaceElement *Level::GetFeatureWallElement(int pFeatureId, size_t pVertex) const
{
	return mFeatureList[pFeatureId].mWallTexture[pVertex].get();
}

SurfaceElement *Level::GetRoomBottomElement(int pRoomId) const
{
	return mRoomList[pRoomId].mFloorTexture.get();
}

SurfaceElement *Level::GetFeatureBottomElement(int pFeatureId) const
{
	return mFeatureList[pFeatureId].mFloorTexture.get();
}

SurfaceElement *Level::GetRoomTopElement(int pRoomId) const
{
	return mRoomList[pRoomId].mCeilingTexture.get();
}

SurfaceElement *Level::GetFeatureTopElement(int pFeatureId) const
{
	return mFeatureList[pFeatureId].mCeilingTexture.get();
}

// FreeElements manipulation
MR_FreeElementHandle Level::GetFirstFreeElement(int pRoom) const
{
	FreeElementList *lReturnValue;

	if(pRoom == eNonClassified)
		lReturnValue = mFreeElementNonClassifiedList;
	else
		lReturnValue = mFreeElementClassifiedByRoomList[pRoom];

	return (MR_FreeElementHandle) lReturnValue;
}

MR_FreeElementHandle Level::GetNextFreeElement(MR_FreeElementHandle pHandle)
{
	return (MR_FreeElementHandle) ((FreeElementList *) pHandle)->mNext;
}

FreeElement *Level::GetFreeElement(MR_FreeElementHandle pHandle)
{
	return ((FreeElementList *) pHandle)->mElement.get();
}

void Level::MoveElement(MR_FreeElementHandle pHandle, int pNewRoom)
{
	if(pNewRoom == eNonClassified) {
		((FreeElementList *) pHandle)->LinkTo(&mFreeElementNonClassifiedList);
	}
	else {
		((FreeElementList *) pHandle)->LinkTo(&(mFreeElementClassifiedByRoomList[pNewRoom]));
	}
}

MR_FreeElementHandle Level::InsertElement(std::shared_ptr<FreeElement> pElement,
	int pRoom, BOOL pBroadcast)
{
	FreeElementList *lReturnValue = new FreeElementList;

	if(mAllowRendering) {
		pElement->AddRenderer();
	}

	lReturnValue->mElement = pElement;

	MoveElement((MR_FreeElementHandle) lReturnValue, pRoom);

	// Broadcast element creation if needed
	if(pBroadcast && (mElementCreationBroadcastHook != NULL)) {
		mElementCreationBroadcastHook(lReturnValue->mElement.get(), pRoom,
			mBroadcastHookData);
	}
	return (MR_FreeElementHandle) lReturnValue;
}

void Level::DeleteElement(MR_FreeElementHandle pHandle)
{
	delete((FreeElementList *) pHandle);
}

MR_FreeElementHandle Level::GetPermanentElementHandle(int pElem) const
{

	ASSERT(pElem >= 0);
	ASSERT(pElem < mNbPermNetActor);

	return (MR_FreeElementHandle) mPermNetActor[pElem];
}

void Level::SetPermElementPos(int pPermElement, int pRoom, const MR_3DCoordinate & pNewPos)
{
	if(pPermElement >= 0) {
		ASSERT(pPermElement < mNbPermNetActor);

		// MoveElement( (MR_FreeElementHandle)mPermNetActor[ pPermElement ], pRoom );

		mPermNetActor[pPermElement]->mElement->mPosition = pNewPos;
		if (mPermElementStateBroadcastHook) {
			mPermElementStateBroadcastHook(
				mPermNetActor[pPermElement]->mElement.get(),
				pRoom, pPermElement, mBroadcastHookData);
		}
		// Insert the element in the moving cache
		mPermActorIndexCache[mPermActorCacheCount] = pPermElement;
		mPermActorNewRoomCache[mPermActorCacheCount] = pRoom;
		mPermActorCacheCount++;
	}
}

void Level::FlushPermElementPosCache()
{
	for(int lCounter = 0; lCounter < mPermActorCacheCount; lCounter++) {
		MoveElement((MR_FreeElementHandle) mPermNetActor[mPermActorIndexCache[lCounter]], mPermActorNewRoomCache[lCounter]);
	}
	mPermActorCacheCount = 0;

}

void Level::GetRoomContact(int pRoom, const ShapeInterface * pShape, RoomContactSpec & pAnswer)
{

	// Verify if the current room contains the requires shape
	SectionShape room(&(mRoomList[pRoom]));
	DetectRoomContact(pShape, &room, pAnswer);
}

BOOL Level::GetRoomWallContactOrientation(int pRoom, int pWall, const ShapeInterface * pShape, MR_Angle & pAnswer)
{
	SectionShape room(&(mRoomList[pRoom]));
	return GetWallForceLongitude(pShape, &room, pWall, pAnswer);
}

BOOL Level::GetFeatureContact(int pFeature, const ShapeInterface * pShape, ContactSpec & pAnswer)
{

	// Verify if the current room contains the requires shape
	SectionShape feature(&(mFeatureList[pFeature]));
	return DetectFeatureContact(pShape, &feature, pAnswer);
}

BOOL Level::GetFeatureContactOrientation(int pFeature, const ShapeInterface * pShape, MR_Angle & pAnswer)
{
	SectionShape feature(&(mFeatureList[pFeature]));
	return GetFeatureForceLongitude(pShape, &feature, pAnswer);
}

// Sub classes implementation

// class Level::Section::AudibleRoom
Level::Room::AudibleRoom::AudibleRoom() :
	mSectionSource(-1), mNbVertexSources(0),
	mVertexList(nullptr), mSoundCoefficient(nullptr)
{
}

Level::Room::AudibleRoom::~AudibleRoom()
{
	delete[] mVertexList;
	delete[] mSoundCoefficient;
}

void Level::Room::AudibleRoom::Serialize(ObjStream & pArchive)
{
	int lCounter;

	if(pArchive.IsWriting()) {
		pArchive << mNbVertexSources;

		for(lCounter = 0; lCounter < mNbVertexSources; lCounter++) {
			pArchive << mVertexList[lCounter]
				<< mSoundCoefficient[lCounter];
		}
	}
	else {
		ASSERT(mVertexList == NULL);			  // Serialisation permited only once

		// Retrieve data
		pArchive >> mNbVertexSources;

		mVertexList = new int[mNbVertexSources];
		mSoundCoefficient = new BYTE[mNbVertexSources];

		for(lCounter = 0; lCounter < mNbVertexSources; lCounter++) {
			pArchive >> mVertexList[lCounter]
				>> mSoundCoefficient[lCounter];
		}
	}

}

int Level::FindRoomForPoint(const MR_2DCoordinate & pPosition, int pStartingRoom) const
{
	int lReturnValue = -1;

	// Verify if the position is included in the current section
	if(GetPolygonInclusion(SectionShape(&mRoomList[pStartingRoom]), pPosition)) {
		lReturnValue = pStartingRoom;
	}
	else {
		// Verify neighbor rooms
		for(int lCounter = 0; lCounter < mRoomList[pStartingRoom].mNbVertex; lCounter++) {
			int lNeighbor = mRoomList[pStartingRoom].mNeighborList[lCounter];

			if(lNeighbor != -1) {
				if(GetPolygonInclusion(SectionShape(&mRoomList[lNeighbor]), pPosition)) {
					lReturnValue = lNeighbor;
					break;
				} else {
					for(int lNCounter = 0; lNCounter < mRoomList[lNeighbor].mNbVertex; lNCounter++) {
						int lNeighborsNeighbor = mRoomList[lNeighbor].mNeighborList[lNCounter];

						if(lNeighborsNeighbor == pStartingRoom) continue;

						if(lNeighborsNeighbor != -1) {
							if(GetPolygonInclusion(SectionShape(&mRoomList[lNeighborsNeighbor]), pPosition)) {
								lReturnValue = lNeighborsNeighbor;
								break;
							}
						}
					}
				}
			}
		}
	}

	/*
	   if( lReturnValue == -1 )
	   {
	   lReturnValue = pStartingRoom;
	   }
	 */

	return lReturnValue;

}

// class Level::Section

Level::Section::Section() :
	mNbVertex(0), mVertexList(nullptr), mWallLen(nullptr)
{
}

Level::Section::~Section()
{
	delete[] mVertexList;
	delete[] mWallLen;
}

void Level::Section::SerializeStructure(ObjStream & pArchive)
{
	int lCounter;

	if(pArchive.IsWriting()) {
		pArchive << mNbVertex;

		pArchive << mFloorLevel << mCeilingLevel;

		mMin.Serialize(pArchive);
		mMax.Serialize(pArchive);

		// Arrays
		for(lCounter = 0; lCounter < mNbVertex; lCounter++) {
			mVertexList[lCounter].Serialize(pArchive);
			pArchive << mWallLen[lCounter];
		}

	}
	else {
		ASSERT(mVertexList == NULL);			  // Can only be called once

		// Simple data
		pArchive >> mNbVertex;
		pArchive >> mFloorLevel >> mCeilingLevel;

		mMin.Serialize(pArchive);
		mMax.Serialize(pArchive);

		// Arrays
		mVertexList = new MR_2DCoordinate[mNbVertex];
		mWallLen = new MR_Int32[mNbVertex];

		for(lCounter = 0; lCounter < mNbVertex; lCounter++) {
			mVertexList[lCounter].Serialize(pArchive);
			pArchive >> mWallLen[lCounter];
		}
	}

	// Serialize the textures
	ObjectFromFactory::SerializeShared<SurfaceElement>(pArchive, mFloorTexture);
	ObjectFromFactory::SerializeShared<SurfaceElement>(pArchive, mCeilingTexture);

	if (!pArchive.IsWriting()) {
		mWallTexture.resize(static_cast<size_t>(mNbVertex));
	}

	for (size_t i = 0; i < static_cast<size_t>(mNbVertex); i++) {
		ObjectFromFactory::SerializeShared<SurfaceElement>(pArchive,
			mWallTexture[i]);
	}
}

// class Level::Room

Level::Room::Room()
{
	mNbChild = 0;
	mChildList = NULL;
	mNeighborList = NULL;
	mNbVisibleRoom = 0;
	mVisibleRoomList = NULL;
	mNbVisibleSurface = 0;
	mVisibleFloorList = NULL;
	mVisibleCeilingList = NULL;

	mNbAudibleRoom = 0;
	mAudibleRoomList = NULL;

}

Level::Room::~Room()
{
	delete[]mChildList;
	delete[]mNeighborList;
	delete[]mVisibleRoomList;
	delete[]mAudibleRoomList;
	delete[]mVisibleFloorList;
	delete[]mVisibleCeilingList;
}

void Level::Room::SerializeStructure(ObjStream & pArchive)
{
	int lCounter;
	Section::SerializeStructure(pArchive);

	if(pArchive.IsWriting()) {
		pArchive << mNbChild;
		pArchive << mNbVisibleRoom;
		pArchive << mNbVisibleSurface;
		pArchive << mNbAudibleRoom;

		for(lCounter = 0; lCounter < mNbVertex; lCounter++) {
			pArchive << mNeighborList[lCounter];
		}

		for(lCounter = 0; lCounter < mNbChild; lCounter++) {
			pArchive << mChildList[lCounter];
		}

		for(lCounter = 0; lCounter < mNbVisibleRoom; lCounter++) {
												  // List of the room that are visible from the current room
			pArchive << mVisibleRoomList[lCounter];
		}

		for(lCounter = 0; lCounter < mNbVisibleSurface; lCounter++) {
			mVisibleFloorList[lCounter].Serialize(pArchive);
			mVisibleCeilingList[lCounter].Serialize(pArchive);
		}

		for(lCounter = 0; lCounter < mNbAudibleRoom; lCounter++) {
			mAudibleRoomList[lCounter].Serialize(pArchive);
		}
	}
	else {
		pArchive >> mNbChild;
		pArchive >> mNbVisibleRoom;
		pArchive >> mNbVisibleSurface;
		pArchive >> mNbAudibleRoom;

		// Arrays

		mNeighborList = new int[mNbVertex];

		if(mNbChild != 0) {
			mChildList = new int[mNbChild];
		}

		if(mNbVisibleRoom != 0) {
			mVisibleRoomList = new int[mNbVisibleRoom];
		}

		if(mNbVisibleSurface != 0) {
			mVisibleFloorList = new SectionId[mNbVisibleSurface];
			mVisibleCeilingList = new SectionId[mNbVisibleSurface];
		}

		if(mNbAudibleRoom != 0) {
			mAudibleRoomList = new AudibleRoom[mNbAudibleRoom];
		}

		for(lCounter = 0; lCounter < mNbVertex; lCounter++) {
			pArchive >> mNeighborList[lCounter];
		}

		for(lCounter = 0; lCounter < mNbChild; lCounter++) {
			pArchive >> mChildList[lCounter];
		}

		for(lCounter = 0; lCounter < mNbVisibleRoom; lCounter++) {
												  // List of the room that are visible from the current room
			pArchive >> mVisibleRoomList[lCounter];
		}

		for(lCounter = 0; lCounter < mNbVisibleSurface; lCounter++) {
			mVisibleFloorList[lCounter].Serialize(pArchive);
			mVisibleCeilingList[lCounter].Serialize(pArchive);
		}

		for(lCounter = 0; lCounter < mNbAudibleRoom; lCounter++) {
			mAudibleRoomList[lCounter].Serialize(pArchive);
		}
	}
}

// class Level::Feature

void Level::Feature::SerializeStructure(ObjStream & pArchive)
{
	Section::SerializeStructure(pArchive);

	if(pArchive.IsWriting()) {
		pArchive << mParentSectionIndex;
	}
	else {
		pArchive >> mParentSectionIndex;
	}
}

// class Level::FreeElementList

Level::FreeElementList::~FreeElementList()
{
	Unlink();
}

void Level::FreeElementList::Unlink()
{
	if (mNext) {
		mNext->mPrevLink = mPrevLink;
	}

	if (mPrevLink) {
		*mPrevLink = mNext;
	}
	mNext = nullptr;
	mPrevLink = nullptr;
}

void Level::FreeElementList::LinkTo(FreeElementList **pPrevLink)
{
	Unlink();

	mPrevLink = pPrevLink;
	mNext = *mPrevLink;
	*mPrevLink = this;

	if (mNext) {
		mNext->mPrevLink = &mNext;
	}
}

void Level::FreeElementList::SerializeList(ObjStream &pArchive,
	FreeElementList **pListHead)
{
	static std::shared_ptr<ObjectFromFactory> NULL_OBJ;

	if (pArchive.IsWriting()) {
		FreeElementList *lFreeElement = *pListHead;

		while (lFreeElement) {
			auto &elem = lFreeElement->mElement;
			ObjectFromFactory::SerializeShared<FreeElement>(pArchive, elem);

			elem->mPosition.Serialize(pArchive);
			pArchive << elem->mOrientation;

			lFreeElement = lFreeElement->mNext;
		}

		ObjectFromFactory::SerializeShared<ObjectFromFactory>(pArchive, NULL_OBJ);
	}
	else {
		assert(*pListHead == nullptr);

		std::shared_ptr<FreeElement> newElem;

		do {
			ObjectFromFactory::SerializeShared<FreeElement>(pArchive, newElem);

			if (newElem) {
				FreeElementList *lFreeElement = new FreeElementList();

				newElem->mPosition.Serialize(pArchive);
				pArchive >> newElem->mOrientation;

				lFreeElement->mElement = newElem;
				lFreeElement->LinkTo(pListHead);
			}

		} while (newElem);
	}
}

// class SectionId
void SectionId::Serialize(ObjStream & pArchive)
{
	if(pArchive.IsWriting()) {
		pArchive << (int) mType;
		pArchive << mId;
	}
	else {
		int lType;

		pArchive >> lType;
		pArchive >> mId;

		mType = (eSectionType) lType;
	}
}

}  // namespace Model
}  // namespace HoverRace
