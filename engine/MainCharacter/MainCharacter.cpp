// MainCharacter.cpp
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

#include "StdAfx.h"

#include <math.h>

#include "MainCharacter.h"
#include "../Model/RaceEffects.h"
#include "../Model/FreeElementMovingHelper.h"
#include "../Util/FuzzyLogic.h"

#define MR_NB_HOVER_MODEL 8

// Local types
class MR_MainCharacterState:private MR_BitPack
{
	friend class MR_MainCharacter;

	// Packing description
	//                     Offset  len   Prec
#define  MC_POSX           0,   32,     5
#define  MC_POSY          32,   32,     5
#define  MC_POSZ          64,   27,     0
#define  MC_ROOM          91,   11,     0
#define  MC_ORIENTATION  102,    9,     3
#define  MC_SPEED_X_256  111,   17,     2
#define  MC_SPEED_Y_256  128,   17,     2
#define  MC_SPEED_Z_256  145,    9,     2
#define  MC_CONTROL_ST   154,   15,     0
#define  MC_ON_FLOOR     169,    1,     0
#define  MC_HOVER_MODEL  170,    3,     0
#define  MC_PADDING      173,   11,     0
	//   #define  MC_SOUNDFX      141,    5,     0

	// Total                 184  = 23 bytes
	MR_UInt8 mFieldList[22];

	public:

};

// Local constants
#define TIME_SLICE                     5
#define MINIMUM_SPLITTABLE_TIME_SLICE  6

const int eCharacterMovementRay = 1100;
const int eCharacterRay = 1300;
const int eCharacterHeight = 1500;
const int eCharacterContactRay = 1450;
const int eCharacterWeight[MR_NB_HOVER_MODEL] = { 300, 250, 450, 300, 300, 300, 300, 300 };
const int eMissileRefillTime = 10000;
const int ePwrUpDuration = 5000;

const double eSteadySpeed[MR_NB_HOVER_MODEL] =
{
	8.7 * 2222.0 / 1000.0,
	11.5 * 2222.0 / 1000.0,
	10.5 * 2222.0 / 1000.0,
	8.7 * 2222.0 / 1000.0,
	8.7 * 2222.0 / 1000.0,
	8.7 * 2222.0 / 1000.0,
	8.7 * 2222.0 / 1000.0,
	8.1 * 2222.0 / 1000.0
};
const double eMaxZSpeed[MR_NB_HOVER_MODEL] =
{
	2900.0 / 1000.0,
	2900.0 / 900.0,
	2900.0 / 1000.0,
	2900.0 / 1000.0,
	2900.0 / 1000.0,
	2900.0 / 1000.0,
	2900.0 / 1000.0,
	2900.0 / 1000.0
};

const double eRotationSpeed = (MR_PI / 1.4) / 1000.0;

const double eFrictionAccell[MR_NB_HOVER_MODEL] =
{
	-eSteadySpeed[0] / 4.0 / 1000.0,
	-eSteadySpeed[0] / 4.0 / 1200.0,
	-eSteadySpeed[0] / 4.0 / 900.0,
	-eSteadySpeed[0] / 4.0 / 1000.0,
	-eSteadySpeed[0] / 4.0 / 1000.0,
	-eSteadySpeed[0] / 4.0 / 1000.0,
	-eSteadySpeed[0] / 4.0 / 1000.0,
	-eSteadySpeed[0] / 4.0 / 700.0
};

const double eMotorAccell[MR_NB_HOVER_MODEL] =
{
	eSteadySpeed[0] / 1000.0,
	eSteadySpeed[0] / 1400.0,
	eSteadySpeed[2] / 1050.0,
	eSteadySpeed[0] / 1000.0,
	eSteadySpeed[0] / 1000.0,
	eSteadySpeed[0] / 1000.0,
	eSteadySpeed[0] / 1000.0,
	eSteadySpeed[7] / 750.0
};

const double eZAccell[MR_NB_HOVER_MODEL] =
{
	-eMaxZSpeed[0] / 1000.0,
	-eMaxZSpeed[1] / 1000.0,
	-eMaxZSpeed[2] / 850.0,
	-eMaxZSpeed[0] / 1000.0,
	-eMaxZSpeed[0] / 1000.0,
	-eMaxZSpeed[0] / 1000.0,
	-eMaxZSpeed[0] / 1000.0,
	-eMaxZSpeed[7] / 1000.0
};

// const double eBreakAccell             = -eSteadySpeed*3.0/1000.0;
// const double eFrictionRotationAccell  = -eSteadyRotationSpeed*6.0/1000.0;
// const double eBreakRotationSpeedAccell= -eSteadyRotationSpeed*12.0/1000.0;
// const double eJetRotationSpeedAccell  =  eSteadyRotationSpeed*12.0/1000.0;

const double eFuelCapacity = 3 * 60 * 1000;		  // 3 minutes of fuel
const double eFuelConsuming[MR_NB_HOVER_MODEL] =
{
	1.0,
	0.70,
	2.0,
	1.0,
	1.0,
	1.0,
	1.0,
	1.1,
};

// Functions implementations

MR_MainCharacter::MR_MainCharacter(const MR_ObjectFromFactoryId & pId)
:MR_FreeElement(pId)
{
	mMasterMode = TRUE;
	mRoom = -1;
	mHoverModel = 0;							  // Basic model
	mNetPriority = FALSE;
	mLastCollisionTime = 0;

	mRenderer = NULL;

	mControlState = 0;
	mMotorOnState = FALSE;
	mMotorDisplay = 0;

	mXSpeed = 0;
	mYSpeed = 0;
	mZSpeed = 0;
	mXSpeedBeforeCollision = 0;
	mYSpeedBeforeCollision = 0;
	mOnFloor = FALSE;
	mCabinOrientation = 0;						  // mOrientation; mOrientation is not set yet
	mOutOfControlDuration = 0;
	mMissileRefillDuration = 0;

	mFireDone = FALSE;

	mCurrentWeapon = eMissile;
	mPowerUpLeft = 0;

	mFuelLevel = eFuelCapacity;

	mCollisionShape.mRay = eCharacterRay;
	mContactShape.mRay = eCharacterContactRay;

	mContactEffectList.AddTail(&mContactEffect);

	mNbLapForRace = 5;							  // Hard coded default
	mLapCount = 0;
	mLastLapCompletion = 0;
	mLastLapDuration = 0;
	mBestLapDuration = 0;

	mHoverId = 10;

	mCheckPoint1 = FALSE;
	mCheckPoint2 = FALSE;

}

MR_MainCharacter::~MR_MainCharacter()
{
	delete mRenderer;
}

void MR_MainCharacter::SetAsMaster()
{
	mMasterMode = TRUE;
}

void MR_MainCharacter::SetAsSlave()
{
	mMasterMode = FALSE;
}

void MR_MainCharacter::SetHoverId(int pId)
{
	mHoverId = pId;
}

void MR_MainCharacter::SetHoverModel(int pModel)
{
	mHoverModel = pModel;
}

int MR_MainCharacter::GetHoverModel() const
{
	return mHoverModel;
} 

void MR_MainCharacter::SetOrientation(MR_Angle pOrientation)
{
	mOrientation = pOrientation;
	mCabinOrientation = pOrientation;
}

int MR_MainCharacter::GetHoverId() const
{
	return mHoverId;
} 

void MR_MainCharacter::AddRenderer()
{
	if(mRenderer == NULL) {
		MR_ObjectFromFactoryId lId = { 1, 100 };
		mRenderer = (MR_MainCharacterRenderer *) MR_DllObjectFactory::CreateObject(lId);
	}
}

void MR_MainCharacter::Render(MR_3DViewPort * pDest, MR_SimulationTime /*pTime */ )
{
	if(mRenderer != NULL)
		mRenderer->Render(pDest, mPosition, mCabinOrientation, mMotorDisplay > 0, mHoverId, mHoverModel);
}

MR_ObjectFromFactory *MR_MainCharacter::FactoryFunc(MR_UInt16)
{
	MR_ObjectFromFactoryId lId = { MR_MAIN_CHARACTER_DLL_ID, MR_MAIN_CHARACTER_CLASS_ID };

	return new MR_MainCharacter(lId);
}

void MR_MainCharacter::RegisterFactory()
{
	MR_DllObjectFactory::RegisterLocalDll(MR_MAIN_CHARACTER_DLL_ID, FactoryFunc);
}

MR_MainCharacter *MR_MainCharacter::New(int pNbLap, char pGameOpts)
{
	MR_ObjectFromFactoryId lId = { MR_MAIN_CHARACTER_DLL_ID, MR_MAIN_CHARACTER_CLASS_ID };

	MR_MainCharacter *lReturnValue = (MR_MainCharacter *) MR_DllObjectFactory::CreateObject(lId);

	if(lReturnValue != NULL) {
		lReturnValue->mNbLapForRace = pNbLap;
		lReturnValue->mGameOpts = pGameOpts;
	}
	// if mGameOpts now outlaw basic craft, we must update
	while(!(lReturnValue->mGameOpts & ((int) pow(2.0f, 3.0f - ((lReturnValue->mHoverModel + 4) % 4)))))
		lReturnValue->mHoverModel++;

	return lReturnValue;
}

MR_ElementNetState MR_MainCharacter::GetNetState() const
{
	static MR_MainCharacterState lsState;		  // Static is ok because the variable will be used immediatly

	MR_ElementNetState lReturnValue;

	lReturnValue.mDataLen = sizeof(lsState);
	lReturnValue.mData = (MR_UInt8 *) &lsState;

	lsState.Clear(sizeof(lsState));

	lsState.Set(MC_POSX, mPosition.mX);
	lsState.Set(MC_POSY, mPosition.mY);
	lsState.Set(MC_POSZ, mPosition.mZ);

	lsState.Set(MC_ROOM, mRoom);

	lsState.Set(MC_ORIENTATION, mOrientation);

	lsState.Set(MC_SPEED_X_256, static_cast<unsigned int>(mXSpeedBeforeCollision * 256));
	lsState.Set(MC_SPEED_Y_256, static_cast<unsigned int>(mYSpeedBeforeCollision * 256));
	lsState.Set(MC_SPEED_Z_256, static_cast<unsigned int>(mZSpeed * 256));

	lsState.Set(MC_CONTROL_ST, mControlState);
	lsState.Set(MC_ON_FLOOR, mOnFloor);
	lsState.Set(MC_HOVER_MODEL, mHoverModel);
	lsState.Set(MC_PADDING, 0);					  // no sounds now

	/*
	   lsState.mPosX          = mPosition.mX;
	   lsState.mPosY          = mPosition.mY;
	   lsState.mPosZ          = mPosition.mZ;

	   // lsState.mRoom          = mRoom;

	   lsState.mOrientation   = mOrientation;

	   lsState.mXSpeed_256    = mXSpeed*256;
	   lsState.mYSpeed_256    = mYSpeed*256;
	   lsState.mZSpeed_256    = mZSpeed*256;

	   lsState.mControlState  = mControlState;

	   lsState.mOnFloor       = mOnFloor;
	 */
	return lReturnValue;
}

void MR_MainCharacter::SetNetState(int /*pDataLen */ , const MR_UInt8 *pData)
{
	const MR_MainCharacterState *lState = (const MR_MainCharacterState *) pData;

	mPosition.mX = lState->Get(MC_POSX);
	mPosition.mY = lState->Get(MC_POSY);
	mPosition.mZ = lState->Get(MC_POSZ);

	mRoom = lState->Get(MC_ROOM);

	if(mRoom < -1)
		mRoom = lState->Getu(MC_ROOM);

	mOrientation = lState->Getu(MC_ORIENTATION);

	mXSpeed = lState->Get(MC_SPEED_X_256) / 256.0;
	mYSpeed = lState->Get(MC_SPEED_Y_256) / 256.0;
	mZSpeed = lState->Get(MC_SPEED_Z_256) / 256.0;

	// calculate actual speed
	double totalSpeed = sqrt(mXSpeed * mXSpeed + mYSpeed * mYSpeed);
	//TRACE("set player %d speed to %lf\n", this->GetHoverId(), totalSpeed);

	mControlState = lState->Getu(MC_CONTROL_ST);
	mOnFloor = lState->Get(MC_ON_FLOOR);
	mHoverModel = lState->Getu(MC_HOVER_MODEL);

	/*
	   mPosition.mX = lState->mPosX;
	   mPosition.mY = lState->mPosY;
	   mPosition.mZ = lState->mPosZ;

	   // mRoom        = lState->mRoom;

	   mOrientation = lState->mOrientation;

	   mXSpeed      = lState->mXSpeed_256/256.0;
	   mYSpeed      = lState->mYSpeed_256/256.0;
	   mZSpeed      = lState->mZSpeed_256/256.0;

	   mControlState= lState->mControlState;

	   mOnFloor     = lState->mOnFloor;
	 */

	// Determine cabin orientation
	if(mControlState & eBreakDirection)
		mCabinOrientation = MR_NORMALIZE_ANGLE(RAD_2_MR_ANGLE(atan2(mYSpeed, mXSpeed)) + MR_PI);
	//else if( (mControlState & eStraffleRight)^(mControlState & eStraffleLeft)) {
	//      if(mControlState & eStraffleRight)
	//              mCabinOrientation = MR_NORMALIZE_ANGLE(mOrientation - MR_PI / 2);
	//      else
	//              mCabinOrientation = MR_NORMALIZE_ANGLE(mOrientation + MR_PI / 2);
	//}
}

void MR_MainCharacter::SetNbLapForRace(int pNbLap)
{
	mNbLapForRace = pNbLap;
}

void MR_MainCharacter::SetControlState(int pState, MR_SimulationTime pTime)
{
	int lState = pState;

	// Set HoverType if race not started
	if(pTime < 0) {
		if(!(mControlState & (eRight | eLeft))) {
			if(pState & eRight) {
				mHoverModel++;
				
				// ensure we are using an allowed craft
				while(!(mGameOpts & ((int) pow(2.0f, 3.0f - ((mHoverModel + 4) % 4)))))
					mHoverModel++;
			}

			if(pState & eLeft) {
				mHoverModel--;

				// ensure we are using an allowed craft
				while(!(mGameOpts & ((int) pow(2.0f, 3.0f - ((mHoverModel + 4) % 4)))))
					mHoverModel--;
			}

			mHoverModel = (mHoverModel + 4) % 4;
		}
	}
	// First verify transition states
	if(!(mControlState & eSelectWeapon) && (lState & eSelectWeapon)) {
		(*(int *) &mCurrentWeapon)++;
		if(mCurrentWeapon == eNotAWeapon)
			(*(int *) &mCurrentWeapon) = 0;
	}
	if(!(mControlState & eFire) && (lState & eFire))
		mFireDone = FALSE;

	if(!(mControlState & eJump) && (lState & eJump)) {
		if(mOnFloor) {
			mZSpeed = 1.1 * eMaxZSpeed[mHoverModel];
			if(mRenderer != NULL)
				mInternalSoundList.Add(mRenderer->GetJumpSound());
		}
		else {
			if(mRenderer != NULL)
				mInternalSoundList.Add(mRenderer->GetMisJumpSound());
		}
	}

	// We're no longer looking behind us
	if((mControlState & eLookBack) && !(lState & eLookBack))
		mOrientation = mCabinOrientation;

	if(!(mControlState & (eRight | eLeft)))
		lState |= eSlowRotation;

	// Now verify continous states
	if(mFuelLevel <= 0.0) {
		if((!mMotorOnState) && (lState & eMotorOn))
			mFuelLevel = 120;
		else
			lState &= ~eMotorOn;
	}
	mMotorOnState = pState & eMotorOn;
	mControlState = lState;
}

int MR_MainCharacter::Simulate(MR_SimulationTime pDuration, MR_Level *pLevel, int pRoom)
{
	mRoom = pRoom;

	if(pDuration > 0) {
		if(mMasterMode) {
			if((mControlState & eMotorOn) && (mFuelLevel > 0.0))
				mMotorDisplay = 250;
		}
		else {
			if(mControlState & eMotorOn)
				mMotorDisplay = 250;
		}
	}

	mMotorDisplay -= pDuration;

	if(mMotorDisplay < 0)
		mMotorDisplay = 0;

	// Orient the cabin if a special move have been made
	if(mControlState & eBreakDirection) {
		double lAbsoluteSpeed = sqrt(mXSpeed * mXSpeed + mYSpeed * mYSpeed);
		if(lAbsoluteSpeed > (eSteadySpeed[mHoverModel] / 20.0))
			mCabinOrientation = MR_NORMALIZE_ANGLE(RAD_2_MR_ANGLE(atan2(mYSpeed, mXSpeed)) + MR_PI);
	}
	else if(mControlState & eLookBack) {
		if(mCabinOrientation - mOrientation < (MR_PI / 2))
			mOrientation = MR_NORMALIZE_ANGLE(mCabinOrientation - MR_PI);
	}
	//else if((mControlState & eStraffleRight) ^ (mControlState & eStraffleLeft)) {
	//      if(mControlState & eStraffleRight)
	//              mCabinOrientation = MR_NORMALIZE_ANGLE(mOrientation - MR_PI / 2);
	//      else
	//              mCabinOrientation = MR_NORMALIZE_ANGLE(mOrientation + MR_PI / 2);
	//}
	else
		mCabinOrientation = mOrientation;

	if(mMasterMode) {
		MR_SimulationTime lDuration = pDuration;

		while(lDuration > 0) {
			if(lDuration > TIME_SLICE)
				pRoom = InternalSimulate(TIME_SLICE, pLevel, pRoom);
			else
				pRoom = InternalSimulate(lDuration, pLevel, pRoom);
			lDuration -= TIME_SLICE;
		}

		mXSpeedBeforeCollision = mXSpeed;
		mYSpeedBeforeCollision = mYSpeed;

		// If the user pressed fire, launch a missile
		mMissileRefillDuration -= pDuration;
		if(mMissileRefillDuration <= 0)
			mMissileRefillDuration = 0;

		mPowerUpLeft -= static_cast<MR_SimulationTime>(pDuration * eFuelConsuming[mHoverModel]);
		if(mPowerUpLeft < 0)
			mPowerUpLeft = 0;

		if((mControlState & eFire) && !mFireDone) {
			mFireDone = TRUE;

			if(mCurrentWeapon == eMissile) {
				if((mMissileRefillDuration == 0) && (mGameOpts & OPT_ALLOW_WEAPONS)) {
					mMissileRefillDuration = eMissileRefillTime;

					MR_ObjectFromFactoryId lObjectId = { 1, 150 };
					// Create a new missile
					MR_FreeElement *lMissile = (MR_FreeElement *) MR_DllObjectFactory::CreateObject(lObjectId);

					if(lMissile != NULL) {
						lMissile->SetOwnerId(mHoverId);
						lMissile->mPosition = mPosition;
						lMissile->mPosition.mZ += 1100;
						lMissile->mOrientation = mCabinOrientation;

						pLevel->InsertElement(lMissile, mRoom, TRUE);

						if(mRenderer != NULL) {
							mInternalSoundList.Add(mRenderer->GetFireSound());
							mExternalSoundList.Add(mRenderer->GetFireSound());
						}
					}
				}
			}
			else if(mCurrentWeapon == eMine) {
				if(!mMineList.IsEmpty() && (mGameOpts & OPT_ALLOW_MINES)) {
					MR_3DCoordinate lPos = mPosition;
					lPos.mZ += 800;
					pLevel->SetPermElementPos(mMineList.GetHead(), mRoom, lPos);
					mMineList.Remove();
				}
			}
			else if(mCurrentWeapon == ePowerUp) {
				if(!mPowerUpList.IsEmpty() && (mGameOpts & OPT_ALLOW_CANS)) {
					MR_3DCoordinate lPos = mPosition;
					lPos.mZ += 1200;

					pLevel->SetPermElementPos(mPowerUpList.GetHead(), mRoom, lPos);
					mPowerUpList.Remove();

					mPowerUpLeft = ePwrUpDuration;
				}
			}
		}
	} else									  // Slave mode
		pRoom = InternalSimulate(pDuration, pLevel, pRoom);
	
	return pRoom;
}

int MR_MainCharacter::InternalSimulate(MR_SimulationTime pDuration, MR_Level *pLevel, int pRoom)
{
	// Determine new speed (PosVar and OrientationVar
	double lAbsoluteSpeed = sqrt(mXSpeed * mXSpeed + mYSpeed * mYSpeed);

	if(mOutOfControlDuration <= 0) {		  // No friction when out of control..it's just more cool
		if(lAbsoluteSpeed <= -pDuration * eFrictionAccell[mHoverModel]) {
			mXSpeed = 0;
			mYSpeed = 0;
		}
		else {
			double lFrictionAmplifier = 1.0;

			if(lAbsoluteSpeed < eSteadySpeed[mHoverModel] / 3)
				lFrictionAmplifier = 0.4;
			else if((lAbsoluteSpeed > eSteadySpeed[mHoverModel]) && (lAbsoluteSpeed < 2.5 * eSteadySpeed[mHoverModel]))
				lFrictionAmplifier = min(1.7, 1.1 + 2.5 * (lAbsoluteSpeed / eSteadySpeed[0] - 1.0));

			double lConstantPart = pDuration * lFrictionAmplifier * eFrictionAccell[mHoverModel] / lAbsoluteSpeed;

			mXSpeed += lConstantPart * mXSpeed;
			mYSpeed += lConstantPart * mYSpeed;
		}
	}

	mZSpeed += pDuration * eZAccell[mHoverModel];

	if(mZSpeed < -eMaxZSpeed[mHoverModel])
		mZSpeed = -eMaxZSpeed[mHoverModel];

	// Apply rotation
	if(mOutOfControlDuration > 0) {
		mOutOfControlDuration -= pDuration;

		mOrientation = MR_NORMALIZE_ANGLE((int) (mOrientation + pDuration * 8 * eRotationSpeed));
	}
	else {
		if((mControlState & eRight) ^ (mControlState & eLeft)) {
			double lRotation;

			if(mControlState & eRight)
				lRotation = -pDuration * eRotationSpeed;
			else
				lRotation = pDuration * eRotationSpeed;

			if(mControlState & eSlowRotation)
				lRotation /= 4;

			if(mControlState & eLookBack)
				mCabinOrientation = MR_NORMALIZE_ANGLE(mCabinOrientation + (int) lRotation);
			else
				mOrientation = MR_NORMALIZE_ANGLE(mOrientation + (int) lRotation);
		}
	}

	// MotorEffect
	if(mControlState & eMotorOn) {
		double lDirectionalSpeed = (mXSpeed * MR_Cos[mCabinOrientation] + mYSpeed * MR_Sin[mCabinOrientation]) / MR_TRIGO_FRACT;

		double lMaxSpeedFactor = 1.3;

		if(mPowerUpLeft > 0)
			lMaxSpeedFactor = 1.9;

		if(lDirectionalSpeed < (lMaxSpeedFactor * eSteadySpeed[mHoverModel])) {
			double lAccelerationFactor = 1.0 - lDirectionalSpeed / (lMaxSpeedFactor * 1.25 * eSteadySpeed[mHoverModel]);

			if(lAccelerationFactor < 0)
				lAccelerationFactor = 0;
			else if(lAccelerationFactor > 0.8)
				lAccelerationFactor = 0.8;

			if(mPowerUpLeft > 0)
				lAccelerationFactor *= 3.2;
			else
				lAccelerationFactor *= 1.8;

			mXSpeed += (pDuration * lAccelerationFactor * eMotorAccell[mHoverModel] * MR_Cos[mCabinOrientation]) / MR_TRIGO_FRACT;
			mYSpeed += (pDuration * lAccelerationFactor * eMotorAccell[mHoverModel] * MR_Sin[mCabinOrientation]) / MR_TRIGO_FRACT;
		}

		mFuelLevel -= pDuration * eFuelConsuming[mHoverModel];
	}
	// Determine new dispacement
	Cylinder lShape;

	lShape.mRay = eCharacterMovementRay;	  //eCharacterRay;

	// Compute speed objectives
	MR_3DCoordinate lTranslation((mXSpeed * (int) pDuration), (mYSpeed * (int) pDuration), (mZSpeed * (int) pDuration));

	// This avoid continious jumping
	if(lTranslation.mZ == 0)
		lTranslation.mZ = -1;

	// Verify if the move is valid
	MR_ObstacleCollisionReport lReport;

	BOOL lSuccessfullTry;
	MR_SimulationTime lDuration = pDuration;

	lShape.mPosition.mX = mPosition.mX + lTranslation.mX;
	lShape.mPosition.mY = mPosition.mY + lTranslation.mY;
	lShape.mPosition.mZ = mPosition.mZ + lTranslation.mZ;

	while(1) {
		lSuccessfullTry = FALSE;

		lReport.GetContactWithObstacles(pLevel, &lShape, pRoom, this);
		if(lReport.IsInMaze()) {
			if(!lReport.HaveContact()) {
				mPosition = lShape.mPosition;
				pRoom = lReport.Room();
				mOnFloor = FALSE;

				lSuccessfullTry = TRUE;
			} else {
				// Determine if we can go on the object
				if((lReport.SpaceToCeiling() > 0) && (lReport.StepHeight() <= 1 - lTranslation.mZ)) {
					lShape.mPosition.mZ += lReport.StepHeight();
					lReport.GetContactWithObstacles(pLevel, &lShape, pRoom, this);

					if(lReport.IsInMaze()) {
						if(!lReport.HaveContact()) {
							if((mRenderer != NULL) && (!mOnFloor))
								mInternalSoundList.Add(mRenderer->GetBumpSound());

							mZSpeed = 0;
							mOnFloor = TRUE;

							mPosition = lShape.mPosition;
							pRoom = lReport.Room();

							lSuccessfullTry = TRUE;
						}
					}
				}
				else if((mZSpeed > 0) && (lReport.SpaceToFloor() > 0) && (lReport.CeilingStepHeight() <= lTranslation.mZ)) {
					lShape.mPosition.mZ -= lReport.CeilingStepHeight() + 1;

					lReport.GetContactWithObstacles(pLevel, &lShape, pRoom, this);

					if(lReport.IsInMaze()) {
						if(!lReport.HaveContact())
							mZSpeed = 0;
						mOnFloor = FALSE;

						mPosition = lShape.mPosition;
						pRoom = lReport.Room();

						lSuccessfullTry = TRUE;
					}
				}
			}
		}

		if(lDuration < MINIMUM_SPLITTABLE_TIME_SLICE)
			break;
		else {
			if(lSuccessfullTry) {
				if(lDuration == pDuration)
					break;					  // success on first attempt

				lDuration /= 2;

				lShape.mPosition.mX += lDuration * lTranslation.mX / pDuration;
				lShape.mPosition.mY += lDuration * lTranslation.mY / pDuration;
				lShape.mPosition.mZ += lDuration * lTranslation.mZ / pDuration;
			} else {
				lDuration /= 2;

				lShape.mPosition.mX -= lDuration * lTranslation.mX / pDuration;
				lShape.mPosition.mY -= lDuration * lTranslation.mY / pDuration;
				lShape.mPosition.mZ -= lDuration * lTranslation.mZ / pDuration;
			}
		}
	}

	if((mFuelLevel < 0.0) && mMasterMode)
		mControlState &= ~eMotorOn;

	mRoom = pRoom;
	return pRoom;
}

const MR_ShapeInterface *MR_MainCharacter::GetObstacleShape()
{
	return NULL;
	mCollisionShape.mPosition = mPosition;
	return &mCollisionShape;
}

void MR_MainCharacter::ApplyEffect(const MR_ContactEffect * pEffect, MR_SimulationTime pTime, MR_SimulationTime pDuration, BOOL pValidDirection, MR_Angle pHorizontalDirection, MR_Int32 /*pZMin */ ,
	MR_Int32 pZMax, MR_Level * pLevel)
{
	MR_ContactEffect *lEffect = (MR_ContactEffect *) pEffect;
	const MR_PhysicalCollision *lPhysCollision = dynamic_cast < MR_PhysicalCollision * >(lEffect);
	const MR_SpeedDoubler *lSpeedDoubler = dynamic_cast < MR_SpeedDoubler * >(lEffect);
	const MR_FuelGain *lFuelGain = dynamic_cast < MR_FuelGain * >(lEffect);
	const MR_LostOfControl *lLostOfControl = dynamic_cast < MR_LostOfControl * >(lEffect);
	const MR_CheckPoint *lLapCompleted = dynamic_cast < MR_CheckPoint * >(lEffect);
	const MR_PowerUpEffect *lPowerUp = dynamic_cast < MR_PowerUpEffect * >(lEffect);

	if((lPhysCollision != NULL) && pValidDirection) {
		/*
		   double lInvertDampingFactor = 1.0;
		   double lDampingFactor       = 0.0;

		   if( pZMax-210 < mPosition.mZ )
		   {
		   lInvertDampingFactor = (pZMax-mPosition.mZ)/210;
		   lDampingFactor       = 1.0-lInvertDampingFactor;

		   mZSpeed = max( mZSpeed, 512*lDampingFactor*-eZAccell[mHoverModel] );
		   }
		 */

		if(pValidDirection) {
			MR_InertialMoment lMoment;

			lMoment.mWeight = eCharacterWeight[mHoverModel];
			lMoment.mXSpeed = mXSpeed * 256;
			lMoment.mYSpeed = mYSpeed * 256;
			lMoment.mZSpeed = 0;

			lMoment.ComputeCollision(lPhysCollision, pHorizontalDirection);

			// mXSpeed = (lInvertDampingFactor*lMoment.mXSpeed/256.0)+lDampingFactor*mXSpeed;
			// mYSpeed = (lInvertDampingFactor*lMoment.mYSpeed/256.0)+lDampingFactor*mYSpeed;
			mXSpeed = lMoment.mXSpeed / 256.0;
			mYSpeed = lMoment.mYSpeed / 256.0;

			if((mRenderer != NULL) && !((lMoment.mXSpeed == 0) && (lMoment.mYSpeed == 0))) {
				mInternalSoundList.Add(mRenderer->GetBumpSound());
				mExternalSoundList.Add(mRenderer->GetBumpSound());
			}

			if((lPhysCollision->mXSpeed != 0 || lPhysCollision->mYSpeed != 0)) {
				TRACE("Net priority increase %d\n", mMasterMode);
				mNetPriority = TRUE;
				mLastCollisionTime = pTime;
			}
		}
	}

	if(lSpeedDoubler != NULL) {
		/*
		   mXSpeed *= 2;
		   mYSpeed *= 2;

		   double lAbsSpeed =  sqrt( mXSpeed*mXSpeed+mYSpeed*mYSpeed );

		   if( lAbsSpeed > eSteadySpeed/4 )
		   {
		mXSpeed = eSteadySpeed*2*mXSpeed/lAbsSpeed;
		mYSpeed = eSteadySpeed*2*mYSpeed/lAbsSpeed;
		   }
		 */

		mXSpeed = (4 * eSteadySpeed[0] * MR_Cos[mCabinOrientation]) / MR_TRIGO_FRACT;
		mYSpeed = (4 * eSteadySpeed[0] * MR_Sin[mCabinOrientation]) / MR_TRIGO_FRACT;
	}

	if((lFuelGain != NULL) && mMasterMode) {
		mFuelLevel += pDuration * lFuelGain->mFuelQty;

		if(mFuelLevel > eFuelCapacity) {
			mFuelLevel = eFuelCapacity;
		}
	}

	if((lLostOfControl != NULL) && mMasterMode) {
		if(mOutOfControlDuration < 1750)
			mLastHits.Add(lLostOfControl->mHoverId);

		mOutOfControlDuration = 2000;

		if(mRenderer != NULL) {
			mInternalSoundList.Add(mRenderer->GetOutOfCtrlSound());
			mExternalSoundList.Add(mRenderer->GetOutOfCtrlSound());
		}

		if(lLostOfControl->mType == MR_LostOfControl::eMine) {
			mZSpeed = 1.1 * eMaxZSpeed[0];

			if((lLostOfControl->mElementId != -1) && !mMineList.Full() && (mGameOpts & OPT_ALLOW_MINES)) {
				mMineList.Add(lLostOfControl->mElementId);
				pLevel->SetPermElementPos(lLostOfControl->mElementId, -1, mPosition);
			}
		}
	}

	if(((lPowerUp != NULL) && mMasterMode) && (mGameOpts & OPT_ALLOW_CANS)) {
		// TODO Add sound
		if((mPowerUpLeft == 0) && (lPowerUp->mElementPermId != -1) && !mPowerUpList.Full()) {
			mPowerUpList.Add(lPowerUp->mElementPermId);
			pLevel->SetPermElementPos(lPowerUp->mElementPermId, -1, mPosition);
		}
	}

	if((lLapCompleted != NULL) && mMasterMode && (mLapCount < mNbLapForRace)) {
		switch (lLapCompleted->mType) {
			case MR_CheckPoint::eCheck1:
				mCheckPoint1 = TRUE;
				mCheckPoint2 = FALSE;
				break;
			case MR_CheckPoint::eCheck2:
				if(mCheckPoint1)
					mCheckPoint2 = TRUE;
				break;
			case MR_CheckPoint::eFinishLine:
				if(mCheckPoint2) {
					mCheckPoint1 = FALSE;
					mCheckPoint2 = FALSE;

					mLapCount++;
					mLastLapDuration = pTime - mLastLapCompletion;
					mLastLapCompletion = pTime;

					if((mLastLapDuration < mBestLapDuration) || (mLapCount == 1))
						mBestLapDuration = mLastLapDuration;

					if(mRenderer != NULL) {
						if(mLapCount == mNbLapForRace) {
							mInternalSoundList.Add(mRenderer->GetFinishSound());
							mExternalSoundList.Add(mRenderer->GetFinishSound());
						} else {
							mInternalSoundList.Add(mRenderer->GetLineCrossingSound());
							mExternalSoundList.Add(mRenderer->GetLineCrossingSound());
						}
					}
				}
				break;
		}
	}
}

const MR_ContactEffectList *MR_MainCharacter::GetEffectList()
{
	mContactEffect.mWeight = eCharacterWeight[mHoverModel];
	mContactEffect.mXSpeed = mXSpeed * 256;
	mContactEffect.mYSpeed = mYSpeed * 256;
	mContactEffect.mZSpeed = 0;

	return &mContactEffectList;
}

const MR_ShapeInterface *MR_MainCharacter::GetReceivingContactEffectShape()
{
	mCollisionShape.mPosition = mPosition;
	return &mCollisionShape;
}

const MR_ShapeInterface *MR_MainCharacter::GetGivingContactEffectShape()
{
	mContactShape.mPosition = mPosition;
	return &mContactShape;
}

// MR_MainCharacter::Cylinder
MR_Int32 MR_MainCharacter::Cylinder::ZMin() const
{
	return mPosition.mZ;
}

MR_Int32 MR_MainCharacter::Cylinder::ZMax() const
{
	return mPosition.mZ + eCharacterHeight;
}

MR_Int32 MR_MainCharacter::Cylinder::AxisX() const
{
	return mPosition.mX;
}

MR_Int32 MR_MainCharacter::Cylinder::AxisY() const
{
	return mPosition.mY;
}

MR_Int32 MR_MainCharacter::Cylinder::RayLen() const
{
	return mRay;
}

// Done with the cylinder stuff
MR_Angle MR_MainCharacter::GetCabinOrientation() const
{
	return mCabinOrientation;
}

double MR_MainCharacter::GetFuelLevel() const
{
	return mFuelLevel / eFuelCapacity;
}

MR_MainCharacter::eWeapon MR_MainCharacter::GetCurrentWeapon() const
{
	return mCurrentWeapon;
}

int MR_MainCharacter::GetMissileRefillLevel(int pNbLevel) const
{
	if(mGameOpts & OPT_ALLOW_WEAPONS)
		return (pNbLevel - 1) * (eMissileRefillTime - mMissileRefillDuration) / eMissileRefillTime;
	else
		return 0;
}

int MR_MainCharacter::GetMineCount() const
{
	return mMineList.Used();
}

int MR_MainCharacter::GetPowerUpCount() const
{
	return mPowerUpList.Used();
}

int MR_MainCharacter::GetPowerUpFraction(int pNbLevel) const
{
	int lReturnValue = 0;
	if(mPowerUpLeft > 0) {
		lReturnValue = 1 + ((mPowerUpLeft - 1) * pNbLevel / ePwrUpDuration);
		if(lReturnValue > pNbLevel)
			lReturnValue = pNbLevel;
	}
	return lReturnValue;
}

double MR_MainCharacter::GetAbsoluteSpeed() const
{
	double lReturnValue = sqrt(mXSpeed * mXSpeed + mYSpeed * mYSpeed) / (eSteadySpeed[0] * 1.9);
	if(lReturnValue > 1.0)
		lReturnValue = 1.0;
	return lReturnValue;
}

double MR_MainCharacter::GetDirectionalSpeed() const
{
	double lReturnValue = (mXSpeed * MR_Cos[mCabinOrientation] + mYSpeed * MR_Sin[mCabinOrientation]) / (MR_TRIGO_FRACT * eSteadySpeed[0] * 1.9);

	if(lReturnValue > 1.0)
		lReturnValue = 1.0;
	else if(lReturnValue < -1.0)
		lReturnValue = -1.0;

	return lReturnValue;
}

int MR_MainCharacter::GetLap() const
{
	return mLapCount;
}

int MR_MainCharacter::GetTotalLap() const
{
	return mNbLapForRace;
}

MR_SimulationTime MR_MainCharacter::GetTotalTime() const
{
	return mLastLapCompletion;
}

MR_SimulationTime MR_MainCharacter::GetBestLapDuration() const
{
	return mBestLapDuration;
}

MR_SimulationTime MR_MainCharacter::GetLastLapDuration() const
{
	return mLastLapDuration;
}

MR_SimulationTime MR_MainCharacter::GetLastLapCompletion() const
{
	return mLastLapCompletion;
}

BOOL MR_MainCharacter::HasFinish() const
{
	return (mLapCount >= mNbLapForRace);
}

int MR_MainCharacter::HitQueueCount() const
{
	return mLastHits.Used();
}

int MR_MainCharacter::GetHitQueue()
{
	int lReturnValue = mLastHits.GetHead();
	mLastHits.Remove();
	return lReturnValue;
}

void MR_MainCharacter::PlayInternalSounds()
{
	if(mRenderer != NULL) {
		// Sound events
		while(!mInternalSoundList.IsEmpty()) {
			MR_SoundServer::Play(mInternalSoundList.GetHead());
			mInternalSoundList.Remove();
		}

		// Continuous sounds
		MR_ContinuousSound *lWindSound = mRenderer->GetFrictionSound();
		MR_ContinuousSound *lMotorSound = mRenderer->GetMotorSound();
		double lAbsSpeed = sqrt(mXSpeed * mXSpeed + mYSpeed * mYSpeed) / (eSteadySpeed[0]);

		if(lAbsSpeed > 0.02)
			MR_SoundServer::Play(lWindSound, 0, 0, 1.5 * lAbsSpeed);
		if(mControlState & eMotorOn)
			MR_SoundServer::Play(lMotorSound, 0);
	}
}

void MR_MainCharacter::PlayExternalSounds(int pDB, int pPan)
{
	if(mRenderer != NULL) {
		// Sound events
		while(!mExternalSoundList.IsEmpty()) {
			MR_SoundServer::Play(mExternalSoundList.GetHead(), pDB, 1.0, pPan);
			mExternalSoundList.Remove();
		}

		// Continuous sounds
		MR_ContinuousSound *lWindSound = mRenderer->GetFrictionSound();
		MR_ContinuousSound *lMotorSound = mRenderer->GetMotorSound();
		double lAbsSpeed = sqrt(mXSpeed * mXSpeed + mYSpeed * mYSpeed) / (eSteadySpeed[0]);

		if(lAbsSpeed > 0.02)
			MR_SoundServer::Play(lWindSound, 1, pDB, 1.5 * lAbsSpeed, pPan);
		if(mControlState & eMotorOn)
			MR_SoundServer::Play(lMotorSound, 1, pDB, 1.0, pPan);
	}
}