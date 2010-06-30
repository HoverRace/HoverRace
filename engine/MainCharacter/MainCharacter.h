// MainCharacter.h
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

#pragma once

#include "MainCharacterRenderer.h"
#include "../Model/MazeElement.h"
#include "../Model/PhysicalCollision.h"
#include "../Util/FastFifo.h"
#include "../Util/BitPacking.h"

#ifdef MR_ENGINE
#define MR_DllDeclare   __declspec( dllexport )
#else
#define MR_DllDeclare   __declspec( dllimport )
#endif

namespace HoverRace {
namespace MainCharacter {

#define MR_MAIN_CHARACTER_DLL_ID     5000
#define MR_MAIN_CHARACTER_CLASS_ID      1

// for game options (also found in TrackSelect.h)
#define OPT_ALLOW_WEAPONS	0x40
#define OPT_ALLOW_MINES		0x20
#define OPT_ALLOW_CANS		0x10
#define OPT_ALLOW_BASIC		0x08
#define OPT_ALLOW_BI		0x02
#define OPT_ALLOW_CX		0x04
#define OPT_ALLOW_EON		0x01

class MainCharacter : public Model::FreeElement
{
	public:
		enum {
			eMotorOn = 1,
			eRight = 2,
			eLeft = 4,
			eStraffleRight = 8,
			eLookBack = 8,						  // Control added 12/1/2006; eStraffleRight seems unused
			eStraffleLeft = 16,
			eBreakDirection = 32,
			eJump = 64,
			eFire = 128,
			eSlowRotation = 1024,				  // Used internally
			eSelectWeapon = 2048
		};

		enum eWeapon
		{
			eMissile,
			eMine,
			ePowerUp,
			eNotAWeapon
		};
	private:
		class Cylinder:public MR_CylinderShape
		{
			public:
				MR_Int32 mRay;
				MR_3DCoordinate mPosition;

				MR_Int32 ZMin() const;
				MR_Int32 ZMax() const;
				MR_Int32 AxisX() const;
				MR_Int32 AxisY() const;
				MR_Int32 RayLen() const;
		};

	public:
		// Position complement
		int mRoom;

		// Network helper
		BOOL mNetPriority;						  // Indicate that the hover position must
		// be resent as soon as possible due to a collision
		// with a moving object (used only in server mode)

		MR_SimulationTime mLastCollisionTime;	  // Time of the last collision with a moving object
		// Used to ignore out-of date refresh messages
		// This time is only used in slave mode

	private:
		BOOL mMasterMode;
		unsigned mHoverModel;					  // HoverRace model
		MainCharacterRenderer *mRenderer;
		unsigned int mControlState;
		BOOL mMotorOnState;
		int mMotorDisplay;
		char mGameOpts;

		double mXSpeed;
		double mYSpeed;
		double mZSpeed;
		double mXSpeedBeforeCollision;
		double mYSpeedBeforeCollision;

		BOOL mOnFloor;
		MR_Angle mCabinOrientation;
		MR_SimulationTime mOutOfControlDuration;  // Countdown

		BOOL mFireDone;

		eWeapon mCurrentWeapon;
		MR_SimulationTime mMissileRefillDuration;  // Countdown
		MR_FixedFastFifo < int, 2 > mMineList;
		MR_FixedFastFifo < int, 4 > mPowerUpList;
		MR_SimulationTime mPowerUpLeft;

		double mFuelLevel;

		MR_PhysicalCollision mContactEffect;
		MR_ContactEffectList mContactEffectList;
		Cylinder mCollisionShape;
		Cylinder mContactShape;

		int mHoverId;

		// Race stats
		int mNbLapForRace;
		int mLapCount;
		MR_SimulationTime mLastLapCompletion;
		MR_SimulationTime mLastLapDuration;
		MR_SimulationTime mBestLapDuration;

		BOOL mCheckPoint1;
		BOOL mCheckPoint2;

		MR_FixedFastFifo < int, 6 > mLastHits;

		// Sound events list
		MR_FixedFastFifo < HoverRace::VideoServices::ShortSound *, 6 > mInternalSoundList;
		MR_FixedFastFifo < HoverRace::VideoServices::ShortSound *, 6 > mExternalSoundList;

		MainCharacter(const Util::ObjectFromFactoryId & pId);

		static Util::ObjectFromFactory *FactoryFunc(MR_UInt16 pElemenType);

		int InternalSimulate(MR_SimulationTime pDuration, Model::Level * pLevel, int pRoom);

	public:
		// Construction
		MR_DllDeclare static void RegisterFactory();
		MR_DllDeclare static MainCharacter *New(int pNbLap, char pGameOpts);

		~MainCharacter();

		MR_DllDeclare void SetAsMaster();
		MR_DllDeclare void SetAsSlave();

		MR_DllDeclare void SetHoverId(int pId);
		MR_DllDeclare int GetHoverId() const;

		MR_DllDeclare void SetHoverModel(int pModel);
		MR_DllDeclare int GetHoverModel() const;

		MR_DllDeclare void SetOrientation(MR_Angle pOrientation);

		void AddRenderer();
		void Render(MR_3DViewPort * pDest, MR_SimulationTime pTime);

		MR_DllDeclare Model::ElementNetState GetNetState() const;
		MR_DllDeclare void SetNetState(int pDataLen, const MR_UInt8 * pData);
		MR_DllDeclare void SetNbLapForRace(int pNbLap);

		// Movement inputs
		MR_DllDeclare void SetControlState(int pState, MR_SimulationTime pTime);

		// State interogation functions
		MR_DllDeclare MR_Angle GetCabinOrientation() const;

		MR_DllDeclare double GetFuelLevel() const;
		MR_DllDeclare double GetAbsoluteSpeed() const;
		MR_DllDeclare double GetDirectionalSpeed() const;

		MR_DllDeclare eWeapon GetCurrentWeapon() const;
		MR_DllDeclare int GetMissileRefillLevel(int pNbLevel) const;
		MR_DllDeclare int GetMineCount() const;
		MR_DllDeclare int GetPowerUpCount() const;
		MR_DllDeclare int GetPowerUpFraction(int pNbLevel) const;

		MR_DllDeclare int GetLap() const;
		MR_DllDeclare int GetTotalLap() const;
		MR_DllDeclare MR_SimulationTime GetTotalTime() const;
		MR_DllDeclare MR_SimulationTime GetBestLapDuration() const;
		MR_DllDeclare MR_SimulationTime GetLastLapDuration() const;
		MR_DllDeclare MR_SimulationTime GetLastLapCompletion() const;
		MR_DllDeclare BOOL HasFinish() const;

		MR_DllDeclare int HitQueueCount() const;
		MR_DllDeclare int GetHitQueue();

	protected:
		// Logic interface
		int Simulate(MR_SimulationTime pDuration, Model::Level * pLevel, int pRoom);

		const MR_ShapeInterface *GetObstacleShape();

		// ContactEffectShapeInterface
		void ApplyEffect(const MR_ContactEffect * pEffect, MR_SimulationTime pTime, MR_SimulationTime pDuration, BOOL pValidDirection, MR_Angle pHorizontalDirection, MR_Int32 pZMin, MR_Int32 pZMax, Model::Level * pLevel);

		const MR_ContactEffectList *GetEffectList();

		const MR_ShapeInterface *GetReceivingContactEffectShape();
		const MR_ShapeInterface *GetGivingContactEffectShape();

	public:
		// Sounds
		void PlayInternalSounds();
		void PlayExternalSounds(int pDB, int pPan);
};

}  // namespace MainCharacter
}  // namespace HoverRace

#undef MR_DllDeclare
