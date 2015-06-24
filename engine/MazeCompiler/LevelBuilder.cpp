// LevelBuilder.cpp
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

#include <math.h>

#include <boost/filesystem/fstream.hpp>

#include "TrackCompileExn.h"
#include "TrackSpecParser.h"

#include "LevelBuilder.h"

#ifndef _WIN32
#	include <strings.h>
#	define _stricmp strcasecmp
#endif

using namespace HoverRace::Util;
using boost::str;
using boost::format;

namespace HoverRace {
namespace MazeCompiler {

// Helper structurtes
struct MR_Connection
{
	int mRoom0;
	int mWall0;
	int mRoom1;
	int mWall1;
};

// Helper variables
static LevelBuilder *gsCurrentLevelBuilder = nullptr;

// Local helper functions
static Model::SurfaceElement *sLoadTexture(TrackSpecParser * pParser);

LevelBuilder::LevelBuilder(Model::Track &track,
	const TrackCompilationLogPtr &log) :
	SUPER(track), log(log)
{
}

bool LevelBuilder::InitFromFile(const OS::path_t &filename)
{
	boost::filesystem::ifstream in(filename);
	return InitFromStream(in);
}

bool LevelBuilder::InitFromStream(std::istream &in)
{
	return Parse(in) &&
		ComputeVisibleZones() &&
		ComputeAudibleZones() &&
		(OrderVisibleSurfaces(), true);
}

bool LevelBuilder::Parse(std::istream &in)
{
	bool lReturnValue = true;
	int lCounter;
	TrackSpecParser lParser(in);

	typedef std::map<int, int> lRoomList_t;
	lRoomList_t lRoomList;
	typedef std::map<int, int> lFeatureList_t;
	lFeatureList_t lFeatureList;

	// First get the Room and feature count
	mNbRoom = 0;
	mNbFeature = 0;

	while(lReturnValue && (lParser.GetNextClass("Room") != NULL)) {
		if(lParser.GetNextAttrib("Id") == NULL) {
			throw TrackCompileExn(str(format(_("Section ID missing on line %d")) % lParser.GetErrorLine()));
		}
		else {
			int lRoomId = (int) lParser.GetNextNumParam();

			lRoomList_t::const_iterator iter = lRoomList.find(lRoomId);
			if (iter != lRoomList.end()) {
				throw TrackCompileExn(str(
					format(_("Duplicate section ID (%d) on line %d")) %
						lRoomId % lParser.GetErrorLine()));
			}
			else {
				lRoomList[lRoomId] = static_cast<int>(lRoomList.size());
			}
		}
	}

	lParser.Reset();

	while(lReturnValue && (lParser.GetNextClass("Feature") != NULL)) {
		if(lParser.GetNextAttrib("Id") == NULL) {
			throw TrackCompileExn(str(format(_("Section ID missing on line %d")) % lParser.GetErrorLine()));
		}
		else {
			int lFeatureId = (int) lParser.GetNextNumParam();

			lFeatureList_t::const_iterator iter = lFeatureList.find(lFeatureId);
			if (iter != lFeatureList.end()) {
				throw TrackCompileExn(str(
					format(_("Duplicate section ID (%d) on line %d")) %
						lFeatureId % lParser.GetErrorLine()));
			}
			else {
				lFeatureList[lFeatureId] = static_cast<int>(lFeatureList.size());
			}
		}
	}

	if(lReturnValue) {
		// Create the room and feature lists
		mNbRoom = static_cast<int>(lRoomList.size());
		mRoomList = new Room[mNbRoom];

		mNbFeature = static_cast<int>(lFeatureList.size());
		mFeatureList = new Feature[mNbFeature];

		mFreeElementClassifiedByRoomList = new Model::Level::FreeElementList*[mNbRoom];

		for(lCounter = 0; lCounter < mNbRoom; lCounter++) {
			mFreeElementClassifiedByRoomList[lCounter] = NULL;
		}

	}
	// Get the vertex count of each room and feature and allocate the related memory
	lParser.Reset();
	while(lReturnValue && (lParser.GetNextClass("Room") != NULL)) {
		if(lParser.GetNextAttrib("Id") == NULL) {
			ASSERT(FALSE);						  // Already verified
		}
		else {
			int lRoomIndex;
			int lRoomId = (int) lParser.GetNextNumParam();

			// Load the attributes

			lRoomList_t::const_iterator iter = lRoomList.find(lRoomId);
			if (iter == lRoomList.end()) {
				ASSERT(FALSE);
			}
			else {
				lRoomIndex = iter->second;
				const char *lAttrib;

				// Count the number of vertex and find the parent section
				ASSERT(mRoomList[lRoomIndex].mNbVertex == 0);

				while((lAttrib = lParser.GetNextAttrib()) != NULL) {
					if(!_stricmp(lAttrib, "Wall")) {
						mRoomList[lRoomIndex].mNbVertex++;
					}
					else if(!_stricmp(lAttrib, "Floor")) {
						// Load floor attributes
						mRoomList[lRoomIndex].mFloorLevel = (MR_Int32) (lParser.GetNextNumParam() * 1000.0);

						// Create the associated texture
						mRoomList[lRoomIndex].mFloorTexture = sLoadTexture(&lParser);

						if(mRoomList[lRoomIndex].mFloorTexture == NULL) {
							lReturnValue = false;
						}
					}
					else if(!_stricmp(lAttrib, "Ceiling")) {
						// Load floor attributes
						mRoomList[lRoomIndex].mCeilingLevel = (MR_Int32) (lParser.GetNextNumParam() * 1000.0);

						// Create the associated texture
						mRoomList[lRoomIndex].mCeilingTexture = sLoadTexture(&lParser);

						if(mRoomList[lRoomIndex].mCeilingTexture == NULL) {
							lReturnValue = false;
						}
					}
				}
			}
		}
	}

	lParser.Reset();
	while(lReturnValue && (lParser.GetNextClass("Feature") != NULL)) {
		if(lParser.GetNextAttrib("Id") == NULL) {
			ASSERT(FALSE);						  // Already verified
		}
		else {
			int lFeatureIndex;
			int lFeatureId = (int) lParser.GetNextNumParam();

			// Load the attributes

			lFeatureList_t::const_iterator iter = lFeatureList.find(lFeatureId);
			if (iter == lFeatureList.end()) {
				ASSERT(FALSE);
			}
			else {
				lFeatureIndex = iter->second;
				const char *lAttrib;

				// Count the number of vertex and find the parent section
				ASSERT(mFeatureList[lFeatureIndex].mNbVertex == 0);

				while((lAttrib = lParser.GetNextAttrib()) != NULL) {
					if(!_stricmp(lAttrib, "Wall")) {
						mFeatureList[lFeatureIndex].mNbVertex++;
					}
					else if(!_stricmp(lAttrib, "Floor")) {
						// Load floor attributes
						mFeatureList[lFeatureIndex].mFloorLevel = (MR_Int32) (lParser.GetNextNumParam() * 1000.0);

						// Create the associated texture
						mFeatureList[lFeatureIndex].mFloorTexture = sLoadTexture(&lParser);

						if(mFeatureList[lFeatureIndex].mFloorTexture == NULL) {
							lReturnValue = false;
						}
					}
					else if(!_stricmp(lAttrib, "Ceiling")) {
						// Load floor attributes
						mFeatureList[lFeatureIndex].mCeilingLevel = (MR_Int32) (lParser.GetNextNumParam() * 1000.0);

						// Create the associated texture
						mFeatureList[lFeatureIndex].mCeilingTexture = sLoadTexture(&lParser);

						if(mFeatureList[lFeatureIndex].mCeilingTexture == NULL) {
							lReturnValue = false;
						}
					}
					else if(!_stricmp(lAttrib, "Parent")) {
						int lParentId = (int) lParser.GetNextNumParam();

						lRoomList_t::const_iterator iter = lRoomList.find(lParentId);
						if (iter == lRoomList.end()) {
							throw TrackCompileExn(str(
								format(_("Invalid parent room reference on line %d\n")) %
								lParser.GetErrorLine()));
						}
						else {
							mFeatureList[lFeatureIndex].mParentSectionIndex = iter->second;
							mRoomList[mFeatureList[lFeatureIndex].mParentSectionIndex].mNbChild++;
						}
					}
				}
			}
		}
	}

	if(lReturnValue) {

		// Create the arrays that can be created
		for(lCounter = 0; lCounter < mNbRoom; lCounter++) {
			mRoomList[lCounter].mVertexList = new MR_2DCoordinate[mRoomList[lCounter].mNbVertex];
			mRoomList[lCounter].mWallLen = new MR_Int32[mRoomList[lCounter].mNbVertex];

			mRoomList[lCounter].mNeighborList = new int[mRoomList[lCounter].mNbVertex];

			// Init neighbor list
			for(int lNeighbor = 0; lNeighbor < mRoomList[lCounter].mNbVertex; lNeighbor++) {
				mRoomList[lCounter].mNeighborList[lNeighbor] = -1;
			}
			mRoomList[lCounter].mWallTexture = new Model::SurfaceElement *[mRoomList[lCounter].mNbVertex];

			mRoomList[lCounter].mChildList = new int[mRoomList[lCounter].mNbChild];

			// Used the array size as a pointer
			mRoomList[lCounter].mNbChild = 0;

		}

		for(lCounter = 0; lCounter < mNbFeature; lCounter++) {
			mFeatureList[lCounter].mVertexList = new MR_2DCoordinate[mFeatureList[lCounter].mNbVertex];
			mFeatureList[lCounter].mWallLen = new MR_Int32[mFeatureList[lCounter].mNbVertex];

			mFeatureList[lCounter].mWallTexture = new Model::SurfaceElement *[mFeatureList[lCounter].mNbVertex];

			// Create the Parent->Child relations
			int lParentIndex = mFeatureList[lCounter].mParentSectionIndex;
			mRoomList[lParentIndex].mChildList[mRoomList[lParentIndex].mNbChild++] = lCounter;

		}
	}
	// Load the walls related stuff
	lParser.Reset();
	while(lReturnValue && (lParser.GetNextClass("Room") != NULL)) {
		if(lParser.GetNextAttrib("Id") == NULL) {
			ASSERT(FALSE);						  // Already verified
		}
		else {
			int lRoomIndex;
			int lRoomId = (int) lParser.GetNextNumParam();

			// Load the attributes

			lRoomList_t::const_iterator iter = lRoomList.find(lRoomId);
			if (iter == lRoomList.end()) {
				ASSERT(FALSE);
			}
			else {
				lRoomIndex = iter->second;
				int lVertex = 0;

				while(lParser.GetNextAttrib("Wall") != NULL) {
					ASSERT(lVertex < mRoomList[lRoomIndex].mNbVertex);

					// Load the associated position (and convert it from meters to milimeters)
					mRoomList[lRoomIndex].mVertexList[lVertex].mX = (MR_Int32) (lParser.GetNextNumParam() * 1000.0);
					mRoomList[lRoomIndex].mVertexList[lVertex].mY = (MR_Int32) (lParser.GetNextNumParam() * 1000.0);

					// Create the associated texture
					mRoomList[lRoomIndex].mWallTexture[lVertex] = sLoadTexture(&lParser);

					if(mRoomList[lRoomIndex].mWallTexture[lVertex] == NULL) {
						lReturnValue = false;
					}
					lVertex++;
				}
				ASSERT(lVertex >= 3);
			}
		}
	}

	lParser.Reset();
	while(lReturnValue && (lParser.GetNextClass("Feature") != NULL)) {
		if(lParser.GetNextAttrib("Id") == NULL) {
			ASSERT(FALSE);						  // Already verified
		}
		else {
			int lFeatureIndex;
			int lFeatureId = (int) lParser.GetNextNumParam();

			// Load the attributes

			lFeatureList_t::const_iterator iter = lFeatureList.find(lFeatureId);
			if (iter == lFeatureList.end()) {
				ASSERT(FALSE);
			}
			else {
				lFeatureIndex = iter->second;
				int lVertex = 0;

				while(lParser.GetNextAttrib("Wall") != NULL) {
					ASSERT(lVertex < mFeatureList[lFeatureIndex].mNbVertex);

					// Load the associated position (and convert it from meters to milimeters)
					mFeatureList[lFeatureIndex].mVertexList[lVertex].mX = (MR_Int32) (lParser.GetNextNumParam() * 1000.0);
					mFeatureList[lFeatureIndex].mVertexList[lVertex].mY = (MR_Int32) (lParser.GetNextNumParam() * 1000.0);

					// Create the associated texture
					mFeatureList[lFeatureIndex].mWallTexture[lVertex] = sLoadTexture(&lParser);

					if(mFeatureList[lFeatureIndex].mWallTexture[lVertex] == NULL) {
						lReturnValue = false;
					}

					lVertex++;
				}

				ASSERT(lVertex >= 3);
			}
		}
	}

	// Load the connections
	typedef std::vector<MR_Connection> lConnectionList_t;
	lConnectionList_t lConnectionList;

	lParser.Reset();

	if(lParser.GetNextClass("Connection_List") == NULL) {
		throw TrackCompileExn(_("Connection list not found"));
	}
	else {
		while(lParser.GetNextLine()) {
			MR_Connection lNewConnection;

			lNewConnection.mRoom0 = (int) lParser.GetNextNumParam();
			lNewConnection.mWall0 = (int) lParser.GetNextNumParam();
			lNewConnection.mRoom1 = (int) lParser.GetNextNumParam();
			lNewConnection.mWall1 = (int) lParser.GetNextNumParam();

			lConnectionList.push_back(lNewConnection);
		}

		// Assign the connections to the MR_Level
		for (MR_Connection &lConnection : lConnectionList) {
			lRoomList_t::const_iterator iter;

			iter = lRoomList.find(lConnection.mRoom0);
			if (iter == lRoomList.end()) {
				throw TrackCompileExn(str(
					format(_("Connection to a nonexistent section %d")) %
					lConnection.mRoom0));
			}
			int lRoom0 = iter->second;

			iter = lRoomList.find(lConnection.mRoom1);
			if (iter == lRoomList.end()) {
				throw TrackCompileExn(str(
					format(_("Connection to a nonexistent section %d")) %
					lConnection.mRoom1));
			}
			int lRoom1 = iter->second;

			if ((mRoomList[lRoom0].mVertexList[lConnection.mWall0] != mRoomList[lRoom1].mVertexList[(lConnection.mWall1 + 1) % mRoomList[lRoom1].mNbVertex]) ||
				(mRoomList[lRoom1].mVertexList[lConnection.mWall1] != mRoomList[lRoom0].mVertexList[(lConnection.mWall0 + 1) % mRoomList[lRoom0].mNbVertex]))
			{
				log->Warn(_("Mismatched connections"));
				log->Warn(str(format("%3dA: %5d, %5d\n") %
					lConnection.mRoom0 %
					mRoomList[lRoom0].mVertexList[lConnection.mWall0].mX %
					mRoomList[lRoom0].mVertexList[lConnection.mWall0].mY));
				log->Warn(str(format("%3dB: %5d, %5d\n") %
					lConnection.mRoom0 %
					(mRoomList[lRoom0].mVertexList[(lConnection.mWall0 + 1) % mRoomList[lRoom0].mNbVertex].mX) %
					(mRoomList[lRoom0].mVertexList[(lConnection.mWall0 + 1) % mRoomList[lRoom0].mNbVertex].mY)));
				log->Warn(str(format("%3dA: %5d, %5d\n") %
					lConnection.mRoom1 %
					mRoomList[lRoom1].mVertexList[lConnection.mWall1].mX %
					mRoomList[lRoom1].mVertexList[lConnection.mWall1].mY));
				log->Warn(str(format("%3dB: %5d, %5d\n") %
					lConnection.mRoom1 %
					(mRoomList[lRoom1].mVertexList[(lConnection.mWall1 + 1) % mRoomList[lRoom1].mNbVertex].mX) %
					(mRoomList[lRoom1].mVertexList[(lConnection.mWall1 + 1) % mRoomList[lRoom1].mNbVertex].mY)));
				throw TrackCompileExn(str(
					format(_("Connection between sections %d and %d do not match")) %
					lConnection.mRoom0 %
					lConnection.mRoom1));
			}
			else {
				// We can now do the connection
				mRoomList[lRoom0].mNeighborList[lConnection.mWall0] = lRoom1;
				mRoomList[lRoom1].mNeighborList[lConnection.mWall1] = lRoom0;
			}
		}
	}

	// Compute the bounding box of each Room and do some validation
	if(lReturnValue) {
		for (lRoomList_t::value_type &item : lRoomList) {
			int lRoomId = item.first;
			int lRoomIndex = item.second;

			double lDiagSize = ComputeShapeConst(&(mRoomList[lRoomIndex]));

			if(lDiagSize < 10.0) {
				log->Warn(str(format(_("Warning: room %d seems to be very small")) % lRoomId));
			}

			if(lDiagSize > 100.0) {
				log->Warn(str(format(_("Warning: room %d seems to be very large")) % lRoomId));
			}
		}
	}

	if(lReturnValue) {
		for (lFeatureList_t::value_type &item : lFeatureList) {
			int lFeatureId = item.first;
			int lFeatureIndex = item.second;

			double lDiagSize = ComputeShapeConst(&(mFeatureList[lFeatureIndex]));

			if(lDiagSize < 1.0) {
				log->Warn(str(format(_("Warning: feature %d seems to be very small")) % lFeatureId));
			}

			if(lDiagSize > 25.0) {
				log->Warn(str(format(_("Warning: feature %d seems to be very large")) % lFeatureId));
			}
		}
	}

	// Load the player's starting positions
	if(lReturnValue) {
		int lNbStartingPosition = 0;

		lParser.Reset();

		while(lReturnValue && (lParser.GetNextClass("Initial_Position") != NULL)) {
			const char *lAttrib;

			while((lAttrib = lParser.GetNextAttrib()) != NULL) {
				if(!_stricmp(lAttrib, "Section")) {
					int lRoomId = (int) lParser.GetNextNumParam();

					lRoomList_t::const_iterator iter = lRoomList.find(lRoomId);
					if (iter == lRoomList.end()) {
						throw TrackCompileExn(str(
							format(_("Invalid starting position room %d")) % lRoomId));
					}
					else {
						mStartingRoom[lNbStartingPosition] = iter->second;
					}
				}
				else if(!_stricmp(lAttrib, "Position")) {
					mStartingPosition[lNbStartingPosition].mX = (MR_Int32) (lParser.GetNextNumParam() * 1000.0);
					mStartingPosition[lNbStartingPosition].mY = (MR_Int32) (lParser.GetNextNumParam() * 1000.0);
					mStartingPosition[lNbStartingPosition].mZ = (MR_Int32) (lParser.GetNextNumParam() * 1000.0);
				}
				else if(!_stricmp(lAttrib, "Orientation")) {
					mStartingOrientation[lNbStartingPosition] = (MR_Angle) (lParser.GetNextNumParam() * MR_2PI / 360.0);
				}
				else if(!_stricmp(lAttrib, "Team")) {
					mPlayerTeam[lNbStartingPosition] = (int) lParser.GetNextNumParam();
				}
				// Do some validation

				// TODO
			}
			lNbStartingPosition++;
		}

		if(lReturnValue) {
			if(lNbStartingPosition == 0) {
				throw TrackCompileExn(_("No starting positions found"));
			}
		}
		mNbPlayer = lNbStartingPosition;
	}
	// Load the mobile elements
	lParser.Reset();
	int lFreeElementCount = 0;

	while(lReturnValue && (lParser.GetNextClass("Free_Element") != NULL)) {
		int lRoomIndex = 0;
		MR_3DCoordinate lPosition;
		MR_Angle lOrientation = 0;
		Util::ObjectFromFactoryId lElementType;

		lFreeElementCount++;

		// Load the attributes

		const char *lAttrib;

		while((lAttrib = lParser.GetNextAttrib()) != NULL) {
			if(!_stricmp(lAttrib, "Section")) {
				int lRoomId = (int) lParser.GetNextNumParam();

				lRoomList_t::const_iterator iter = lRoomList.find(lRoomId);
				if (iter != lRoomList.end()) {
					throw TrackCompileExn(str(
						format(_("Invalid room index on line %d")) %
						lParser.GetErrorLine()));
				}
				else {
					lRoomIndex = iter->second;
				}
			}
			else if(!_stricmp(lAttrib, "Position")) {
				lPosition.mX = (MR_Int32) (lParser.GetNextNumParam() * 1000.0);
				lPosition.mY = (MR_Int32) (lParser.GetNextNumParam() * 1000.0);
				lPosition.mZ = (MR_Int32) (lParser.GetNextNumParam() * 1000.0);
			}
			else if(!_stricmp(lAttrib, "Orientation")) {
				lOrientation = (MR_Angle) (lParser.GetNextNumParam() * MR_2PI / 360.0);
			}
			else if(!_stricmp(lAttrib, "Element_Type")) {
				lElementType.mDllId = (MR_UInt16) lParser.GetNextNumParam();
				lElementType.mClassId = (MR_UInt16) lParser.GetNextNumParam();
			}
		}

		if(lReturnValue) {

			Model::FreeElement *lElement = (Model::FreeElement*)Util::DllObjectFactory::CreateObject(lElementType);

			if(lElement == NULL) {
				throw TrackCompileExn(str(
					format(_("Unable to create free element on line %d\n")) %
					lParser.GetErrorLine()));
			}
			else {
				// Initialise element position
				lElement->mPosition = lPosition;
				lElement->mOrientation = lOrientation;

				InsertElement(lElement, lRoomIndex);

				// If needed, pass the initialisation string to the element
				// TODO
			}
		}
	}

	// Print some statistics
	if(lReturnValue) {
		log->Info(_("This level contains:"));
		log->Info(str(format(" %6d %s") % lRoomList.size() % _("rooms")));
		log->Info(str(format(" %6d %s") % lFeatureList.size() % _("features")));
		log->Info(str(format(" %6d %s") % lConnectionList.size() % _("connections")));
		log->Info(str(format(" %6d %s") % lFreeElementCount % _("elements")));
		log->Info(str(format(" %6d %s") % mNbPlayer % _("starting positions")));
	}

	return lReturnValue;
}

bool LevelBuilder::ComputeAudibleZones()
{
	// Not implemented.
	return true;
}

void LevelBuilder::OrderVisibleSurfaces()
{
	int lCounter;

	// For each room, compute the list of the visible surfaces
	for(int lRoomId = 0; lRoomId < mNbRoom; lRoomId++) {
		mRoomList[lRoomId].mNbVisibleSurface = 0;

		// Compute the number of visible surfaces

		mRoomList[lRoomId].mNbVisibleSurface = mRoomList[lRoomId].mNbChild + 1;

		for(lCounter = 0; lCounter < mRoomList[lRoomId].mNbVisibleRoom; lCounter++) {
			mRoomList[lRoomId].mNbVisibleSurface += mRoomList[mRoomList[lRoomId].mVisibleRoomList[lCounter]].mNbChild + 1;
		}

		// Create the arrays containing the list of visible floor and ceiling
		int lCurrentIndex = 0;

		mRoomList[lRoomId].mVisibleFloorList = new Model::SectionId[mRoomList[lRoomId].mNbVisibleSurface];
		mRoomList[lRoomId].mVisibleCeilingList = new Model::SectionId[mRoomList[lRoomId].mNbVisibleSurface];

		for(lCounter = -1; lCounter < mRoomList[lRoomId].mNbVisibleRoom; lCounter++) {
			int lVisibleRoom;

			if(lCounter == -1) {
				lVisibleRoom = lRoomId;
			}
			else {
				lVisibleRoom = mRoomList[lRoomId].mVisibleRoomList[lCounter];
			}

			mRoomList[lRoomId].mVisibleFloorList[lCurrentIndex].mType = Model::SectionId::eRoom;
			mRoomList[lRoomId].mVisibleFloorList[lCurrentIndex].mId = lVisibleRoom;
			mRoomList[lRoomId].mVisibleCeilingList[lCurrentIndex].mType = Model::SectionId::eRoom;
			mRoomList[lRoomId].mVisibleCeilingList[lCurrentIndex++].mId = lVisibleRoom;

			for(int lChildIndex = 0; lChildIndex < mRoomList[lVisibleRoom].mNbChild; lChildIndex++) {
				mRoomList[lRoomId].mVisibleFloorList[lCurrentIndex].mType = Model::SectionId::eFeature;
				mRoomList[lRoomId].mVisibleFloorList[lCurrentIndex].mId = mRoomList[lVisibleRoom].mChildList[lChildIndex];
				mRoomList[lRoomId].mVisibleCeilingList[lCurrentIndex].mType = Model::SectionId::eFeature;
				mRoomList[lRoomId].mVisibleCeilingList[lCurrentIndex++].mId = mRoomList[lVisibleRoom].mChildList[lChildIndex];
			}

			ASSERT(lCurrentIndex <= mRoomList[lRoomId].mNbVisibleSurface);
		}

		ASSERT(lCurrentIndex == mRoomList[lRoomId].mNbVisibleSurface);

		// Order the surfaces list
		gsCurrentLevelBuilder = this;

		auto &room = mRoomList[lRoomId];
		size_t numVisible = static_cast<size_t>(room.mNbVisibleSurface);
		qsort(room.mVisibleFloorList, numVisible,
			sizeof(Model::SectionId), OrderFloor);
		qsort(room.mVisibleCeilingList, numVisible,
			sizeof(Model::SectionId), OrderCeiling);
	}
}

int LevelBuilder::OrderFloor(const void *pSurface0, const void *pSurface1)
{
	int lLevel0;
	int lLevel1;

	Model::SectionId lSurface0 = *(Model::SectionId *) pSurface0;
	Model::SectionId lSurface1 = *(Model::SectionId *) pSurface1;

	if(lSurface0.mType == Model::SectionId::eRoom) {
		lLevel0 = gsCurrentLevelBuilder->mRoomList[lSurface0.mId].mFloorLevel;
	}
	else {
		lLevel0 = gsCurrentLevelBuilder->mFeatureList[lSurface0.mId].mCeilingLevel;
	}

	if(lSurface1.mType == Model::SectionId::eRoom) {
		lLevel1 = gsCurrentLevelBuilder->mRoomList[lSurface1.mId].mFloorLevel;
	}
	else {
		lLevel1 = gsCurrentLevelBuilder->mFeatureList[lSurface1.mId].mCeilingLevel;
	}

	return (lLevel0 - lLevel1);
}

int LevelBuilder::OrderCeiling(const void *pSurface0, const void *pSurface1)
{
	int lLevel0;
	int lLevel1;

	Model::SectionId lSurface0 = *(Model::SectionId *) pSurface0;
	Model::SectionId lSurface1 = *(Model::SectionId *) pSurface1;

	if(lSurface0.mType == Model::SectionId::eFeature) {
		lLevel0 = gsCurrentLevelBuilder->mFeatureList[lSurface0.mId].mFloorLevel;
	}
	else {
		lLevel0 = gsCurrentLevelBuilder->mRoomList[lSurface0.mId].mCeilingLevel;
	}

	if(lSurface1.mType == Model::SectionId::eFeature) {
		lLevel1 = gsCurrentLevelBuilder->mFeatureList[lSurface1.mId].mFloorLevel;
	}
	else {
		lLevel1 = gsCurrentLevelBuilder->mRoomList[lSurface1.mId].mCeilingLevel;
	}

	return (lLevel1 - lLevel0);
}

double LevelBuilder::ComputeShapeConst(Section * pSection)
{
	int lCounter;
	double lReturnValue;

	pSection->mMin.mX = pSection->mVertexList[0].mX;
	pSection->mMax.mX = pSection->mVertexList[0].mX;
	pSection->mMin.mY = pSection->mVertexList[0].mY;
	pSection->mMax.mY = pSection->mVertexList[0].mY;

	for(lCounter = 1; lCounter < pSection->mNbVertex; lCounter++) {
		if(pSection->mMin.mX > pSection->mVertexList[lCounter].mX) {
			pSection->mMin.mX = pSection->mVertexList[lCounter].mX;
		}
		if(pSection->mMax.mX < pSection->mVertexList[lCounter].mX) {
			pSection->mMax.mX = pSection->mVertexList[lCounter].mX;
		}

		if(pSection->mMin.mY > pSection->mVertexList[lCounter].mY) {
			pSection->mMin.mY = pSection->mVertexList[lCounter].mY;
		}

		if(pSection->mMax.mY < pSection->mVertexList[lCounter].mY) {
			pSection->mMax.mY = pSection->mVertexList[lCounter].mY;
		}
	}

	for(lCounter = 0; lCounter < pSection->mNbVertex; lCounter++) {
		int lNext = (lCounter + 1) % pSection->mNbVertex;

		double lXLen = pSection->mVertexList[lNext].mX - pSection->mVertexList[lCounter].mX;

		double lYLen = pSection->mVertexList[lNext].mY - pSection->mVertexList[lCounter].mY;

		pSection->mWallLen[lCounter] = (MR_Int32) sqrt(pow(lXLen, 2) + pow(lYLen, 2));
	}

	// Compute bonding box diag size
	lReturnValue = sqrt(
		pow((float)(pSection->mMax.mX - pSection->mMin.mX), 2.0f) +
		pow((float)(pSection->mMax.mY - pSection->mMin.mY), 2.0f)) / 1000.0;

	return lReturnValue;
}

// Helpers implementation
Model::SurfaceElement *sLoadTexture(TrackSpecParser *pParser)
{
	Model::SurfaceElement *lReturnValue = NULL;
	Util::ObjectFromFactory *lTempPtr = NULL;

	Util::ObjectFromFactoryId lType;

	lType.mDllId = (MR_UInt16) pParser->GetNextNumParam();
	lType.mClassId = (MR_UInt16) pParser->GetNextNumParam();

	lTempPtr = Util::DllObjectFactory::CreateObject(lType);

	if(lTempPtr != NULL) {
		lReturnValue = dynamic_cast < Model::SurfaceElement * >(lTempPtr);

		if(lReturnValue == NULL) {
			lTempPtr = NULL;
		}
	}

	if(lReturnValue == NULL) {
		throw TrackCompileExn(str(
			format(_("Unable to load texture %d %d on line %d\n")) %
				lType.mDllId %
				lType.mClassId %
				pParser->GetErrorLine()));
	}

	return lReturnValue;
}

}  // namespace MazeCompiler
}  // namespace HoverRace
