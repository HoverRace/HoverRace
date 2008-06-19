// LevelBuilderVisiblesZones.cpp
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

#include <math.h>
#include "LevelBuilder.h"
#include "../Util/StrRes.h"
#include "resource.h"

#define new DEBUG_NEW

// Usefull constants
#define MR_MAX_VISIBLE_ZONES   500

static const double Pi = fabs(atan2((double) 0, (double) -1));

// Usefull local structures
class MR_2DFloatPos
{
	public:
		double mX;
		double mY;

		const MR_2DFloatPos & operator=(const MR_2DCoordinate & pPos);
		BOOL operator==(const MR_2DFloatPos & pPos) const;
		BOOL operator!=(const MR_2DFloatPos & pPos) const;

};

class MR_VisibleStep
{
	public:
		MR_VisibleStep * mPrev;

		int mZone;								  // Section Id

		MR_2DFloatPos mLeftNode;
		MR_2DFloatPos mRightNode;

		MR_2DFloatPos mLeftLimit;
		MR_2DFloatPos mRightLimit;

		MR_2DFloatPos mLeftLimitLightSource;
		MR_2DFloatPos mRightLimitLightSource;

		MR_VisibleStep();
		~MR_VisibleStep();

};

// Local functions
static void AddZonetoArray(int pZone, int *pDestArray, int &pDestIndex);
static double GetAngle(const MR_2DFloatPos & pPoint0, const MR_2DFloatPos & pPoint1, double pRef = 0.0);

// Functions implementation
BOOL MR_LevelBuilder::ComputeVisibleZones()
{
	BOOL lReturnValue = TRUE;

	int lDestIndex;
	int lDestArray[MR_MAX_VISIBLE_ZONES];

	printf(MR_LoadString(IDS_COMP_VISZONES));

	MR_VisibleStep lStep;

	for(int lCounter = 0; lCounter < mNbRoom; lCounter++) {
		lDestIndex = 0;							  // Index in destination array

		lStep.mZone = lCounter;

		for(int lCounter2 = 0; lCounter2 < mRoomList[lCounter].mNbVertex; lCounter2++) {
			lStep.mLeftNode = mRoomList[lCounter].mVertexList[lCounter2];
			lStep.mRightNode = mRoomList[lCounter].mVertexList[(lCounter2 + 1) % mRoomList[lCounter].mNbVertex];

			lStep.mLeftLimit = lStep.mLeftNode;
			lStep.mLeftLimitLightSource = lStep.mRightNode;

			lStep.mRightLimit = lStep.mRightNode;
			lStep.mRightLimitLightSource = lStep.mLeftNode;

			// PrintStep( &lStep );
			// gMargin += 3;

			TestForVisibility(&lStep, lDestArray, lDestIndex, lCounter2);

			// gMargin -= 3;
		}
		// printf( "%d visibles zones for zone %d\n", lDestIndex, lCounter );

		if(lDestIndex > 0) {
			mRoomList[lCounter].mNbVisibleRoom = lDestIndex;

			mRoomList[lCounter].mVisibleRoomList = new int[lDestIndex];

			for(int lCounter3 = 0; lCounter3 < lDestIndex; lCounter3++) {
				mRoomList[lCounter].mVisibleRoomList[lCounter3] = lDestArray[lCounter3];
			}

		}

	}

	return lReturnValue;
}

void MR_LevelBuilder::TestForVisibility(MR_VisibleStep * pPreviousStep, int *pDestArray, int &pDestIndex, int pNewLeftNodeIndex)
{

	MR_VisibleStep lCurrentStep;

	lCurrentStep.mPrev = pPreviousStep;

	lCurrentStep.mLeftNode = mRoomList[pPreviousStep->mZone].mVertexList[pNewLeftNodeIndex];
	lCurrentStep.mRightNode = mRoomList[pPreviousStep->mZone].mVertexList[(pNewLeftNodeIndex + 1) % mRoomList[pPreviousStep->mZone].mNbVertex];

	// printf( "%*sNodes %3d %3d\n", gMargin,"",lCurrentStep.mLeftNode, lCurrentStep.mRightNode );

	lCurrentStep.mZone = mRoomList[pPreviousStep->mZone].mNeighborList[pNewLeftNodeIndex];
	if(lCurrentStep.mZone != -1) { {

		// december6-96
		// Verify if the current room is not already in the list
			MR_VisibleStep *lPtr = pPreviousStep;

			while(lPtr != NULL) {
				if(lPtr->mZone == lCurrentStep.mZone) {
					return;
				}
				lPtr = lPtr->mPrev;
			}
		}

		double lAngle;

		// printf( "!opaque " );

		// Verifier si il peut etre atteint par le rayon lumineux

		// Verifier si le point de droite est a l'interieur du champ du point gauche
		if(lCurrentStep.mRightNode == pPreviousStep->mLeftLimitLightSource) {
			lAngle = -1;
		}
		else {
			lAngle = GetAngle(pPreviousStep->mLeftLimit, lCurrentStep.mRightNode, GetAngle(pPreviousStep->mLeftLimitLightSource, pPreviousStep->mLeftLimit));
		}

		// printf( "%*sL%f\n", gMargin,"",lAngle );

		if(lAngle < -0.0001) {
			// Verifier si le point de gauche est a l'interieur du champ du point droit
			if(lCurrentStep.mLeftNode == pPreviousStep->mRightLimitLightSource) {
				lAngle = 1;
			}
			else {
				lAngle = GetAngle(pPreviousStep->mRightLimit, lCurrentStep.mLeftNode, GetAngle(pPreviousStep->mRightLimitLightSource, pPreviousStep->mRightLimit));
			}

			// printf( "%*sR%f\n", gMargin,"", (double)lAngle );

			if(lAngle > 0.0001) {
				// A partir d'ici nous savons que le mur laisse passer la lumiere

				// Finir d'emplir la structure VisibleStep

				// setter les parametres de gauche
				if(pPreviousStep->mLeftLimit == lCurrentStep.mLeftNode) {
					lAngle = 1.0;
				}
				else {
					lAngle = GetAngle(pPreviousStep->mLeftLimit, lCurrentStep.mLeftNode, GetAngle(pPreviousStep->mLeftLimitLightSource, pPreviousStep->mLeftLimit));
				}

				if(lAngle >= 0.0) {
					lCurrentStep.mLeftLimit = pPreviousStep->mLeftLimit;
					lCurrentStep.mLeftLimitLightSource = pPreviousStep->mLeftLimitLightSource;
				}
				else {
					lCurrentStep.mLeftLimit = lCurrentStep.mLeftNode;
					lCurrentStep.mLeftLimitLightSource = lCurrentStep.mRightNode;

					MR_VisibleStep *lStep = pPreviousStep;

					while(lStep != NULL) {
						lAngle = GetAngle(lStep->mRightNode, lCurrentStep.mLeftLimit, GetAngle(lCurrentStep.mLeftLimitLightSource, lCurrentStep.mLeftLimit));
						if(lAngle < 0.0) {
							lCurrentStep.mLeftLimitLightSource = lStep->mRightNode;
						}
						lStep = lStep->mPrev;
					}
				}

				// Setter les parametres de droite
				if(pPreviousStep->mRightLimit == lCurrentStep.mRightNode) {
					lAngle = -1.0;
				}
				else {
					lAngle = GetAngle(pPreviousStep->mRightLimit, lCurrentStep.mRightNode, GetAngle(pPreviousStep->mRightLimitLightSource, pPreviousStep->mRightLimit));
				}

				if(lAngle <= 0.0) {
					lCurrentStep.mRightLimit = pPreviousStep->mRightLimit;
					lCurrentStep.mRightLimitLightSource = pPreviousStep->mRightLimitLightSource;
				}
				else {
					lCurrentStep.mRightLimit = lCurrentStep.mRightNode;
					lCurrentStep.mRightLimitLightSource = lCurrentStep.mLeftNode;

					MR_VisibleStep *lStep = pPreviousStep;

					while(lStep != NULL) {
						lAngle = GetAngle(lStep->mLeftNode, lCurrentStep.mRightLimit, GetAngle(lCurrentStep.mRightLimitLightSource, lCurrentStep.mRightLimit));
						if(lAngle > 0.0) {
							lCurrentStep.mRightLimitLightSource = lStep->mLeftNode;
						}
						lStep = lStep->mPrev;
					}
				}

				// Ajouter la zone courante dans le tableau de points
				AddZonetoArray(lCurrentStep.mZone, pDestArray, pDestIndex);

				// PrintStep( &lCurrentStep );
				// gMargin += 3;

				// Rappeler cette fonction pour tout les mur de la zone courante
				for(int lCounter = 0; lCounter < mRoomList[lCurrentStep.mZone].mNbVertex; lCounter++) {
					MR_2DFloatPos lPos;

					lPos = mRoomList[lCurrentStep.mZone].mVertexList[lCounter];

					if(lPos != lCurrentStep.mRightNode) {

						TestForVisibility(&lCurrentStep, pDestArray, pDestIndex, lCounter);
					}
				}
				// gMargin -= 3;
			}
		}
	}
}

const MR_2DFloatPos & MR_2DFloatPos::operator=(const MR_2DCoordinate & pPos)
{
	mX = pPos.mX / 1000.0;
	mY = pPos.mY / 1000.0;

	return *this;
}

BOOL MR_2DFloatPos::operator==(const MR_2DFloatPos & pPos) const const
{
	return ((mX == pPos.mX) && (mY == pPos.mY));
}

BOOL MR_2DFloatPos::operator!=(const MR_2DFloatPos & pPos) const const
{
	return !(operator==(pPos));
} MR_VisibleStep::MR_VisibleStep()
{
	mPrev = NULL;
}

MR_VisibleStep::~MR_VisibleStep()
{
}

void AddZonetoArray(int pZone, int *pDestArray, int &pDestIndex)
{
	// Verifier si la zone ne fait pas deja parti de l'array
	for(int lCompteur = 0; lCompteur < pDestIndex; lCompteur++) {
		if(pDestArray[lCompteur] == pZone) {
			return;
		}
	}
	pDestArray[pDestIndex++] = pZone;
}

double GetAngle(const MR_2DFloatPos & pPoint0, const MR_2DFloatPos & pPoint1, double pRef)
{

	double ldX = pPoint1.mX - pPoint0.mX;
	double ldY = pPoint1.mY - pPoint0.mY;

	double lReturnValue;

	if(pPoint0 != pPoint1) {
		lReturnValue = atan2(ldY, ldX) - pRef;

		if(lReturnValue < -Pi) {
			lReturnValue += 2.0 * Pi;
		}

		if(lReturnValue > Pi) {
			lReturnValue -= 2.0 * Pi;
		}

	}
	else {
		fprintf(stderr, MR_LoadString(IDS_SAME_POS));
		lReturnValue = Pi;
		exit(0);
	}
	return lReturnValue;
}
