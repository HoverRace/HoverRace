
// MainCharacter.h
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
// Copyright (c) 2013-2015 Michael Imamura.
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

#include "../Display/Color.h"
#include "../Model/MazeElement.h"
#include "../Model/PhysicalCollision.h"
#include "../Util/FastFifo.h"

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
	namespace MainCharacter {
		class MainCharacterRenderer;
	}
	namespace VideoServices {
		class ShortSound;
	}
}

namespace HoverRace {
namespace MainCharacter {

// for game options (also found in TrackSelect.h)
#define OPT_ALLOW_WEAPONS	0x40
#define OPT_ALLOW_MINES		0x20
#define OPT_ALLOW_CANS		0x10
#define OPT_ALLOW_BASIC		0x08
#define OPT_ALLOW_BI		0x02
#define OPT_ALLOW_CX		0x04
#define OPT_ALLOW_EON		0x01

class MR_DllDeclare MainCharacter : public Model::FreeElement
{
public:
	enum : unsigned int {
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
	class Cylinder : public Model::CylinderShape
	{
	public:
		MR_Int32 mRay;
		MR_3DCoordinate mPosition;

		MR_Int32 ZMin() const override;
		MR_Int32 ZMax() const override;
		MR_Int32 AxisX() const override;
		MR_Int32 AxisY() const override;
		MR_Int32 RayLen() const override;
	};

public:
	// Position complement
	int mRoom;

	// Network helper
	BOOL mNetPriority;  // Indicate that the hover position must
	// be resent as soon as possible due to a collision
	// with a moving object (used only in server mode)

	MR_SimulationTime mLastCollisionTime;  // Time of the last collision with a moving object
	// Used to ignore out-of date refresh messages
	// This time is only used in slave mode

private:
	BOOL mMasterMode;
	unsigned mHoverModel;					  // HoverRace model
	std::shared_ptr<MainCharacterRenderer> mRenderer;
	unsigned int mControlState;
	BOOL mMotorOnState;
	int mMotorDisplay;
	int playerIdx;
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
	MR_FixedFastFifo<int, 2> mMineList;
	MR_FixedFastFifo<int, 4> mPowerUpList;
	MR_SimulationTime mPowerUpLeft;

	double mFuelLevel;

	Model::PhysicalCollision mContactEffect;
	Model::ContactEffectList mContactEffectList;
	Cylinder mCollisionShape;
	Cylinder mContactShape;

	int mHoverId;

	// Race stats
	MR_SimulationTime mLastLapCompletion;
	MR_SimulationTime mLastLapDuration;
	MR_SimulationTime mCurrentTime;

	BOOL mCheckPoint1;
	BOOL mCheckPoint2;

	MR_FixedFastFifo<int, 6> mLastHits;

	// Sound events list
	MR_FixedFastFifo<VideoServices::ShortSound*, 6> mInternalSoundList;
	MR_FixedFastFifo<VideoServices::ShortSound*, 6> mExternalSoundList;

	MainCharacter();

	int InternalSimulate(MR_SimulationTime pDuration, Model::Track &track,
		int pRoom);

public:
	// Construction
	static MainCharacter *New(int idx, char pGameOpts);

	virtual ~MainCharacter();

	void SetAsMaster();
	void SetAsSlave();

	void SetHoverId(int pId);
	int GetHoverId() const;

	void SetHoverModel(unsigned int pModel);
	unsigned int GetHoverModel() const;

	void SetOrientation(MR_Angle pOrientation);

	void AddRenderer() override;
	void Render(VideoServices::Viewport3D *pDest,
		MR_SimulationTime pTime) override;

	Model::ElementNetState GetNetState() const override;
	void SetNetState(int pDataLen, const MR_UInt8 * pData) override;

	// Movement inputs
	void SetSimulationTime(MR_SimulationTime pTime);
	void SetEngineState(bool engineState); // TODO: analog
	void SetTurnLeftState(bool leftState); // TODO: analog
	void SetTurnRightState(bool rightState); // TODO: analog
	void SetJump();
	void SetPowerup();
	void SetChangeItem();
	void SetBrakeState(bool brakeState); // TODO: analog? maybe not
	void SetLookBackState(bool lookBackState);

	// State interogation functions
	MR_Angle GetCabinOrientation() const;

	double GetFuelLevel() const;
	double GetAbsoluteSpeed() const;
	double GetDirectionalSpeed() const;

	eWeapon GetCurrentWeapon() const;
	int GetMissileRefillLevel(int pNbLevel) const;
	int GetMineCount() const;
	int GetPowerUpCount() const;
	int GetPowerUpFraction(int pNbLevel) const;

	int GetPlayerIndex() const { return playerIdx; }

	MR_SimulationTime GetTotalTime() const;
	MR_SimulationTime GetLastLapDuration() const;
	MR_SimulationTime GetLastLapCompletion() const;
	bool HasStarted() const;
	void Finish();
	bool HasFinish() const;

	int HitQueueCount() const;
	int GetHitQueue();

protected:
	// Logic interface
	int Simulate(MR_SimulationTime pDuration, Model::Track &track, int pRoom)
		override;

	const Model::ShapeInterface *GetObstacleShape() override;

	// ContactEffectShapeInterface
	void ApplyEffect(const Model::ContactEffect *pEffect,
		MR_SimulationTime pTime, MR_SimulationTime pDuration,
		BOOL pValidDirection, MR_Angle pHorizontalDirection,
		MR_Int32 pZMin, MR_Int32 pZMax, Model::Track &track) override;

	const Model::ContactEffectList *GetEffectList() override;

	const Model::ShapeInterface *GetReceivingContactEffectShape() override;
	const Model::ShapeInterface *GetGivingContactEffectShape() override;

public:
	// Sounds
	void PlayInternalSounds() override;
	void PlayExternalSounds(int pDB, int pPan) override;

public:
	using startedSignal_t = boost::signals2::signal<void(MainCharacter*)>;
	startedSignal_t &GetStartedSignal() { return startedSignal; }

	using finishedSignal_t = boost::signals2::signal<void(MainCharacter*)>;
	finishedSignal_t &GetFinishedSignal() { return finishedSignal; }

	using checkpointSignal_t =  boost::signals2::signal<void(MainCharacter*, int)>;
	checkpointSignal_t &GetCheckpointSignal() { return checkpointSignal; }

	using finishLineSignal_t = boost::signals2::signal<void(MainCharacter*)>;
	finishLineSignal_t &GetFinishLineSignal() { return finishLineSignal; }

private:
	bool started;
	bool finished;
	startedSignal_t startedSignal;
	finishedSignal_t finishedSignal;
	checkpointSignal_t checkpointSignal;
	finishLineSignal_t finishLineSignal;
};

}  // namespace MainCharacter
}  // namespace HoverRace

#undef MR_DllDeclare
