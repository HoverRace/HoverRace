// main.cpp                              MazeDecompiler main file
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
#include "../Util/RecordFile.h"
#include "HoverCadDoc.h"
#include "HoverTypes.h"
#include "LevelBuilder.h"
#include "TrackMap.h"
#include "Parser.h"
#include "TrackCommonStuff.h"
#include "../Util/StrRes.h"
#include "resource.h"
#include "../Model/Shapes.h";

#include <stdio.h>
#include <stdlib.h>
#include <string>

#define new DEBUG_NEW

static void PrintUsage()
{
	printf(MR_LoadString(IDS_USAGE)
		);
}

static BOOL CreateHeader(FILE * pInputFile, CArchive & pDestination);
static BOOL LoadLevel(MR_RecordFile * pInputFile);
static BOOL ReadHeader(MR_RecordFile * pInputFile);
static BOOL AddBackgroundImage(FILE * pInputFile, CArchive & pDestination);

void CreateRoomOrFeature(int nCurrentMode, int nRoomId, int nFeatureId = 0);
HCNode *GetNodeAtPosition(CPoint pPoint);

static CString FormatStr(const char *pSrc);
static MR_UInt8 *PCXRead(FILE * pFile, int &pXRes, int &pYRes);
static MR_UInt8 *LoadBitmap(FILE * pFile);
static MR_UInt8 *LoadPalette(FILE * pFile);

static char gOwner[81];
static int gMajorID = 0;
static int gMinorID = 0;
static unsigned char gKey[50];

static MR_Level *gCurrentLevel;

static CHoverCadDoc *gHoverCadDoc;

HCNodeList gCurrentNodeList;

BOOL LoadRegistry()
{
	BOOL lReturnValue = FALSE;

	// Registration info
	gOwner[0] = 0;
	gMajorID = -1;
	gMinorID = -1;

	// Now verify in the registry if this information can not be retrieved
	HKEY lProgramKey;

	int lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\HoverRace.com\\HoverRace",
		0,
		KEY_EXECUTE,
		&lProgramKey);

	if(lError == ERROR_SUCCESS) {
		lReturnValue = TRUE;

		unsigned long lBufferSize = sizeof(gOwner);

		if(RegQueryValueEx(lProgramKey, "Owner", 0, NULL, (MR_UInt8 *) gOwner, &lBufferSize) != ERROR_SUCCESS) {
			lReturnValue = FALSE;
		}

		lBufferSize = sizeof(gKey);

		if(RegQueryValueEx(lProgramKey, "Key", 0, NULL, (MR_UInt8 *) gKey, &lBufferSize) != ERROR_SUCCESS) {
			lReturnValue = FALSE;
		}
		else if(lBufferSize != 20) {
			lReturnValue = FALSE;
		}

		int lID[3];
		DWORD lIDSize = sizeof(lID);

		if(RegQueryValueEx(lProgramKey, "RegistrationID", 0, NULL, (MR_UInt8 *) lID, &lIDSize) == ERROR_SUCCESS) {
			gMajorID = lID[0];
			gMinorID = lID[1];
		}
		else {
			lReturnValue = FALSE;
		}
	}
	return lReturnValue;
}

int main(int pArgCount, const char **pArgStrings)
{
	BOOL lPrintUsage = FALSE;
	BOOL lError = FALSE;

	pArgCount = 2;
	pArgStrings[1] = "E:\\Balls To This[1-1666].trk";

	printf(MR_LoadString(IDS_INTRO));

	Sleep(1000);
	MR_DllObjectFactory::Init();

	// Analyse the input parameters
	if(pArgCount != 2) {
		lPrintUsage = TRUE;
		printf(MR_LoadString(IDS_BAD_PARAM_COUNT));
		Sleep(4000);
	}

	if(!lError && !lPrintUsage) {
		CFile lOutputFile;

		// Verify that there is at least one ofhtr parameter
		CString lCopyrightNotice = "\x8\rHoverRace track file, (c)GrokkSoft 1997\n\x1a";

		size_t lLastIndex = std::string(pArgStrings[1]).find_last_of("."); 

		lOutputFile.Open((std::string(pArgStrings[1]).substr(0, lLastIndex) + " DECOMPILE.tr").c_str(), CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareExclusive);

		// Compile each level
		printf(MR_LoadString(IDS_COMP_TRACK), pArgStrings[1]);

		// Open the input file
		// FILE *lFile = fopen(pArgStrings[1], "r");

		MR_RecordFile *lFile = new MR_RecordFile;
		if(!lFile->OpenForRead(pArgStrings[1], TRUE)) {
		}

		if(lFile == NULL) {
			lError = TRUE;

			printf(MR_LoadString(IDS_CANT_OPEN_IN));
		}
		// Compile the level
		gHoverCadDoc = new CHoverCadDoc;

		if(!lError) {
			// lOutputFile
			CArchive lArchive(&lOutputFile, CArchive::store);
			
			lError = !ReadHeader(lFile);
			lError = !LoadLevel(lFile);

			int lRoomCount = gCurrentLevel->GetRoomCount();
			int lOldRoom = -1;

			for(int lRoom = 0; lRoom < lRoomCount; lRoom++) {
				// Add Rooms
				CreateRoomOrFeature(IDC_ROOM, lRoom);

				// Add Elements
				MR_FreeElementHandle lCurrentElem = gCurrentLevel->GetFirstFreeElement(lRoom);

				while(lCurrentElem != NULL) {
					MR_FreeElement *lElem = MR_Level::GetFreeElement(lCurrentElem);

					HCNode *lNode = gHoverCadDoc->AddNode(lElem->mPosition.mX, lElem->mPosition.mY, FALSE);
					gHoverCadDoc->AddItem(lNode, GetTypeIndexFromType(lElem->GetTypeId().mClassId, Object));
					lNode->mItem->mDistanceFromFloor = lElem->mPosition.mZ - gCurrentLevel->GetRoomBottomLevel(lRoom);
					lNode->mItem->mOrientation = lElem->mOrientation;

					lCurrentElem = MR_Level::GetNextFreeElement(lCurrentElem);
				}

				// Add Features
				int lFeatureCount = gCurrentLevel->GetFeatureCount(lRoom);
				int lFeatureIndex = 0;

				for (int lFeature = 0; lFeature < lFeatureCount; lFeature++) {
					if(lOldRoom != lRoom) {
						lOldRoom = lRoom;
						lFeatureIndex = 0;
					}

					CreateRoomOrFeature(IDC_FEATURE, lRoom, lFeature);

					lFeatureIndex++;
				}
			}

			

			for(int lPos = 0; lPos < gCurrentLevel->GetPlayerCount(); lPos++) {
				MR_3DCoordinate lCoord = gCurrentLevel->GetStartingPos(lPos);
				
				HCNode *lNode = gHoverCadDoc->AddNode(lCoord.mX, lCoord.mY, FALSE);
				gHoverCadDoc->AddItem(lNode, lPos);
				lNode->mItem->mDistanceFromFloor = lCoord.mZ - gCurrentLevel->GetRoomBottomLevel(gCurrentLevel->GetStartingRoom(lPos));
				lNode->mItem->mOrientation = gCurrentLevel->GetStartingOrientation(lPos) * 1000.0 * 360.0 / MR_2PI;
			}

			gHoverCadDoc->Serialize(lArchive);
		}

		lOutputFile.Close();

		/*
		if(!lError) {
			// Parse the input file
			// and amze the preprocessing
			if(!lNewLevel->InitFromFile(lFile)) {
				lError = TRUE;
				printf(MR_LoadString(IDS_TRACK_CREATION_ERROR));
			}
			rewind(lFile);
		}
		// Add the level to the file
		if(!lError) {
			if(!lOutputFile.BeginANewRecord()) {
				lError = TRUE;
				printf(MR_LoadString(IDS_CANT_ADD_TRACK));
			}
			else {
				CArchive lArchive(&lOutputFile, CArchive::store);

				lNewLevel->Serialize(lArchive);
			}
		}

		if(!lError) {
			// Add the background file
			if(!lOutputFile.BeginANewRecord()) {
				lError = TRUE;
				printf(MR_LoadString(IDS_CANT_ADD_IMAGE));
			}
			else {
				CArchive lArchive(&lOutputFile, CArchive::store);

				lError = !AddBackgroundImage(lFile, lArchive);
			}
			rewind(lFile);
		}

		if(!lError) {
			// Add the Map
			if(!lOutputFile.BeginANewRecord()) {
				lError = TRUE;
				printf(MR_LoadString(IDS_CANT_ADD_MAP));
			}
			else {
				CArchive lArchive(&lOutputFile, CArchive::store);

				int lX0;
				int lX1;
				int lY0;
				int lY1;

				MR_MapSprite lMapSprite;

				lMapSprite.CreateMap(lNewLevel, lX0, lY0, lX1, lY1);

				lArchive << lX0;
				lArchive << lX1;
				lArchive << lY0;
				lArchive << lY1;

				lMapSprite.Serialize(lArchive);

			}
		}

		delete lNewLevel;

		if(!lError) {
			// Print the statistics on the output file
			printf(MR_LoadString(IDS_REC_COUNT), lOutputFile.GetNbRecords());
		}
		*/

		if(lFile != NULL) {
			lFile->Close();
		}
	}

	/*
	if(!lError) {
		MR_RecordFile lOutputFile;
		lOutputFile.ApplyChecksum(pArgStrings[1]);
	}
	*/

	if(lPrintUsage) {
		if(lError) {
			printf(MR_LoadString(IDS_BAD_CMDLN));
		}
		PrintUsage();
	}

	if(lError) {
		Sleep(7000);
	}

	Sleep(2000);
	// printf( "END\n" );

	MR_DllObjectFactory::Clean(FALSE);

	return lError ? 255 : 0;
}

void CreateRoomOrFeature(int nCurrentMode, int nRoomId, int nFeatureId) {
	MR_PolygonShape * lShape;
	int lZMin, lZMax, lFloorTexture, lCeilingTexture, lFeatureId;

	switch (nCurrentMode) {
		case IDC_ROOM:
				lShape = gCurrentLevel->GetRoomShape(nRoomId);
				lZMin = gCurrentLevel->GetRoomBottomLevel(nRoomId);
				lZMax = gCurrentLevel->GetRoomTopLevel(nRoomId);
				lFloorTexture = gCurrentLevel->GetRoomBottomElement(nRoomId)->GetTypeId().mClassId;
				lCeilingTexture = gCurrentLevel->GetRoomTopElement(nRoomId)->GetTypeId().mClassId;
			break;
		case IDC_FEATURE:
				lFeatureId = gCurrentLevel->GetFeature(nRoomId, nFeatureId);

				lShape = gCurrentLevel->GetFeatureShape(lFeatureId);
				lZMin = gCurrentLevel->GetFeatureBottomLevel(lFeatureId);
				lZMax = gCurrentLevel->GetFeatureTopLevel(lFeatureId);
				lFloorTexture = gCurrentLevel->GetFeatureBottomElement(lFeatureId)->GetTypeId().mClassId;
				lCeilingTexture = gCurrentLevel->GetFeatureTopElement(lFeatureId)->GetTypeId().mClassId;
			break;
	}

	CPoint point;

	for(int lVertexI = 0; lVertexI <= lShape->VertexCount(); lVertexI++) {
		int lVertex = lVertexI % lShape->VertexCount();

		point = CPoint(lShape->X(lVertex), lShape->Y(lVertex));

		// MR_SurfaceElement *lElement = gCurrentLevel->GetRoomWallElement(nRoomId, lVertex);

		// TRACE("lElement Type Id %d\n", lElement->mId.mClassId);

		// Verify if we are not near an other point
		HCNode *lNode = GetNodeAtPosition(point);
	
		if(gCurrentNodeList.IsEmpty()) {
			gHoverCadDoc->ClearSelection();
		}
	
		if(lNode == NULL) {
			// Create a new point at the selected position
			if(gCurrentNodeList.GetCount() < MAX_NODE_PER_POLYGON) {
				lNode = gHoverCadDoc->AddNode(point.x, point.y);
			}
		}
	
		if(lNode != NULL) {
			if(!gCurrentNodeList.IsEmpty() && (lNode == gCurrentNodeList.GetHead())) {
				if(gCurrentNodeList.GetCount() >= 3) {
					// select all nodes from the list
					POSITION lPos = gCurrentNodeList.GetHeadPosition();
	
					while(lPos != NULL) {
						gCurrentNodeList.GetNext(lPos)->mSelected = TRUE;
					}
	
					// add the polygon
					gHoverCadDoc->AddPolygon(nCurrentMode == IDC_FEATURE, gCurrentNodeList, lZMin, lZMax, 0, GetTypeIndexFromType(lFloorTexture, FloorTexture), GetTypeIndexFromType(lCeilingTexture, CeilingTexture), TRUE);
					gCurrentNodeList.RemoveAll();
				}
			}
			else if(gCurrentNodeList.GetCount() < MAX_NODE_PER_POLYGON) {
				// Deselect the last entered node
				if(!gCurrentNodeList.IsEmpty()) {
					gCurrentNodeList.GetTail()->mSelected = FALSE;
				}
				// Make the node selected and add it to the list
				gCurrentNodeList.AddTail(lNode);
				lNode->mSelected = TRUE;
				gHoverCadDoc->UpdateAllViews(NULL);
			}
		}
	}

	// Set wall textures
	HCPolygon *lPolygon = gHoverCadDoc->GetPolygonTail();

	POSITION lAnchorPos = lPolygon->mAnchorWallList.GetHeadPosition();

	int lWallTexture, lWallCount = 0;

	while(lAnchorPos != NULL) {
		switch (nCurrentMode) {
			case IDC_ROOM:
				lWallTexture = gCurrentLevel->GetRoomWallElement(nRoomId, lWallCount)->GetTypeId().mClassId;
				break;
			case IDC_FEATURE:
				lWallTexture = gCurrentLevel->GetFeatureWallElement(lFeatureId, lWallCount)->GetTypeId().mClassId;
			break;
		}

		HCAnchorWall *lAnchor = lPolygon->mAnchorWallList.GetNext(lAnchorPos);
		lAnchor->mWallTexture = GetTypeIndexFromType(lWallTexture, WallTexture);

		lWallCount++;
	}

	gHoverCadDoc->UpdateAllViews(NULL);
	gHoverCadDoc->SetModifiedFlag();
}

HCNode *GetNodeAtPosition(CPoint pPoint)
{
	POSITION lPos = gHoverCadDoc->GetNodeListHead();
	HCNode *lBest = NULL;

	while(lPos != NULL) {
		HCNode *lNode = gHoverCadDoc->GetNextNode(lPos);

		CPoint lNodePos = CPoint(lNode->mX, lNode->mY);

		if (lNodePos.x == pPoint.x && lNodePos.y == pPoint.y)
		{
			lBest = lNode;
		}
	}
	return lBest;
}

BOOL CreateHeader(FILE * pInputFile, CArchive & pArchive)
{
	BOOL lReturnValue = TRUE;

	CString lDescription;
	int lSortingOrder = 50;
	int lRegistration = MR_REGISTRED_TRACK;
	MR_Parser lParser(pInputFile);

	// Look in the registry to find the User name and member number

#ifdef _DEBUG
	lSortingOrder = 30;
	lRegistration = MR_FREE_TRACK;
#else
	if(gMajorID == 0) {
		lSortingOrder = 40;
	}
#endif

	if(lParser.GetNextClass("HEADER") == NULL) {
		lReturnValue = FALSE;
		printf("Unable to find [HEADER] section\n");
	}
	else {
		if(!lParser.GetNextAttrib("Description")) {
			lReturnValue = FALSE;
			printf(MR_LoadString(IDS_NO_DESC));
		}
		else {
			lDescription = FormatStr(lParser.GetParams());
		}

	}

	if(lReturnValue) {
		pArchive << (int) MR_MAGIC_TRACK_NUMBER;
		pArchive << (int) 1;					  // version
		pArchive << lDescription;
		pArchive << (int) gMinorID;
		pArchive << (int) gMajorID;
		pArchive << lSortingOrder;
		pArchive << lRegistration;
		if(lRegistration == MR_FREE_TRACK) {
												  // Hehe dont try to modify the file or it will crash :-)
			pArchive << (int) MR_MAGIC_TRACK_NUMBER;
		}
	}

	return lReturnValue;

}

BOOL LoadLevel(MR_RecordFile * pRecordFile)
{
	gCurrentLevel = new MR_Level(FALSE);
	pRecordFile->SelectRecord(1);
	CArchive lArchive(pRecordFile, CArchive::load);

	gCurrentLevel->Serialize(lArchive);

	return TRUE;
}

BOOL ReadHeader(MR_RecordFile * pRecordFile)
{
	BOOL lReturnValue = FALSE;
	int lMagicNumber;
	
	pRecordFile->SelectRecord(0);
	CArchive lArchive(pRecordFile, CArchive::load);
	
	lArchive >> lMagicNumber;
	if(lMagicNumber == MR_MAGIC_TRACK_NUMBER) {
		int lVersion;

		lArchive >> lVersion;

		if(lVersion == 1) {
			int lMinorID;
			int lMajorID;

			CString cs;
			lArchive >> cs;
			lArchive >> lMinorID;
			lArchive >> lMajorID;

			cs.Replace("\r\n","\n");
			cs.Replace("\n", "\r\n");

			gHoverCadDoc->SetDescription(cs);

			BOOL lIDOk = TRUE;

			int lSortingIndex;
			int lRegistrationMode;

			lArchive >> lSortingIndex;
			lArchive >> lRegistrationMode;

			if(lRegistrationMode == MR_FREE_TRACK) {
				lMagicNumber = 1;
				lArchive >> lMagicNumber;

				if(lMagicNumber == MR_MAGIC_TRACK_NUMBER) {
					lReturnValue = TRUE;
				}
			} else
			lReturnValue = TRUE;
		}
	}
	return lReturnValue;
}

CString FormatStr(const char *pSrc)
{
	CString lReturnValue;
	BOOL lEsc = FALSE;

	while(*pSrc != 0) {
		if(lEsc) {
			lEsc = FALSE;

			switch (*pSrc) {
				case 'n':
					lReturnValue += '\n';
					break;

				case '\\':
					lReturnValue += '\\';
					break;
			}
		}
		else {
			if(*pSrc == '\\') {
				lEsc = TRUE;
			}
			else {
				lReturnValue += *pSrc;
			}
		}
		pSrc++;
	}
	return lReturnValue;
}

BOOL AddBackgroundImage(FILE * pInputFile, CArchive & pDestination)
{
	BOOL lReturnValue = TRUE;

	CString lBackFileName;

	MR_Parser lParser(pInputFile);

	if(lParser.GetNextClass("HEADER") == NULL) {
		lReturnValue = FALSE;
		printf(MR_LoadString(IDS_NO_HEADER));
	}
	else {
		if(!lParser.GetNextAttrib("Background")) {
			lReturnValue = FALSE;
			printf(MR_LoadString(IDS_NO_BACKGROUND));
		}
		else {
			lBackFileName = lParser.GetParams();

		}
	}

	if(lReturnValue) {
		FILE *lBackFile = fopen(lBackFileName, "rb");

		if(lBackFile == NULL) {
			lReturnValue = FALSE;
			printf(MR_LoadString(IDS_NO_BACKGROUND));
		}
		else {
			// Extract the palette and image from the input file
			MR_UInt8 *lBitmap = LoadBitmap(lBackFile);
			MR_UInt8 *lPalette = LoadPalette(lBackFile);

			if((lBitmap != NULL) && (lPalette != NULL)) {
				pDestination << (int) MR_RAWBITMAP;
				pDestination.Write(lPalette, MR_BACK_COLORS * 3);
				pDestination.Write(lBitmap, MR_BACK_X_RES * MR_BACK_Y_RES);
			}
			else {
				printf(MR_LoadString(IDS_NO_BACKGROUND));
			}

			delete[]lBitmap;
			delete[]lPalette;

			fclose(lBackFile);
		}
	}
	return lReturnValue;
}

MR_UInt8 *LoadPalette(FILE * pFile)
{
	MR_UInt8 *lReturnValue = new MR_UInt8[MR_BACK_COLORS * 3];

	fseek(pFile, -769, SEEK_END);

	char lMagicNumber = 0;

	fread(&lMagicNumber, 1, 1, pFile);

	if(lMagicNumber != 12) {
		printf(MR_LoadString(IDS_BAD_PAL));
		delete[]lReturnValue;
		lReturnValue = NULL;
	}
	else {
		fread(lReturnValue, 1, MR_BACK_COLORS * 3, pFile);
	}

	return lReturnValue;
}

MR_UInt8 *LoadBitmap(FILE * pFile)
{
	MR_UInt8 *lReturnValue = NULL;

	int pXRes;
	int pYRes;

	lReturnValue = PCXRead(pFile, pXRes, pYRes);

	if(lReturnValue) {
		if((pXRes != MR_BACK_X_RES) || (pYRes != MR_BACK_Y_RES)) {
			printf(MR_LoadString(IDS_BAD_RES));
			delete[]lReturnValue;
			lReturnValue = NULL;
		}
	}

	if(lReturnValue != NULL) {
		// Make the invertion of the image pixels and add the pixel offset
		MR_UInt8 *lOldBuffer = lReturnValue;

		lReturnValue = new MR_UInt8[pXRes * pYRes];

		int lDestIndex = 0;

		for(int lX = 0; lX < pXRes; lX++) {
			MR_UInt8 *lSource = lOldBuffer + lX;

			for(int lY = pYRes - 1; lY >= 0; lY--) {
				if(*lSource >= 128) {
					// To correct a bug of Paint Shop Pro
					*lSource = 0;
				}
				lReturnValue[lDestIndex + lY] = (*lSource) + MR_RESERVED_COLORS_BEGINNING + MR_BASIC_COLORS;
				lSource += pXRes;
			}
			lDestIndex += pYRes;
		}
		delete lOldBuffer;
	}
	return lReturnValue;
}

MR_UInt8 *PCXRead(FILE * pFile, int &pXRes, int &pYRes)
{
	MR_UInt8 *lReturnValue = NULL;

	MR_Int16 lStartX;
	MR_Int16 lStartY;
	MR_Int16 lSizeX;
	MR_Int16 lSizeY;
	MR_Int16 lNbBytesPerLine;

	fseek(pFile, 4, SEEK_SET);
	fread(&lStartX, 2, 1, pFile);
	fread(&lStartY, 2, 1, pFile);
	fread(&lSizeX, 2, 1, pFile);
	fread(&lSizeY, 2, 1, pFile);

	fseek(pFile, 66, SEEK_SET);
	fread(&lNbBytesPerLine, 2, 1, pFile);

	pXRes = lSizeX - lStartX + 1;
	pYRes = lSizeY - lStartY + 1;

	fseek(pFile, 128, SEEK_SET);

	ASSERT(pXRes > 0);
	ASSERT(pYRes > 0);

	lReturnValue = new MR_UInt8[pXRes * pYRes];

	int lOffset = 0;

	for(int lLineNb = 0; lLineNb < pYRes; lLineNb++) {
		for(int lColNb = 0; lColNb < lNbBytesPerLine;) {
			MR_UInt8 lBuffer;
			MR_UInt8 lBuffer2;

			fread(&lBuffer, 1, 1, pFile);

			if((lBuffer & 0xc0) == 0xc0) {
				fread(&lBuffer2, 1, 1, pFile);
				/*
				   lBuffer2++;
				   if( lBuffer2 == 255 )
				   {
				   lBuffer2 = 0;
				   }
				 */

				for(int lCounter = 0; lCounter < (lBuffer & 0x3f); lCounter++) {
					if(lColNb < pXRes) {
						lReturnValue[lOffset++] = lBuffer2;
					}
					lColNb++;
				}
			}
			else {
				if(lColNb < pXRes) {
					/*
					   lBuffer++;
					   if( lBuffer2 == 255 )
					   {
					   lBuffer2 = 0;
					   }
					 */
					lReturnValue[lOffset++] = lBuffer;
				}
				lColNb++;
			}
		}
	}

	return lReturnValue;
}
