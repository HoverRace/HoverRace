// MovementSpec.h
//

// This file contains structures definition that hold the 
// movement capabilities and state of a character



// Related constants
enum MR_MoveType{ eOnPlanSurface, eOnAir }; 

#define MR_TRACTION_MAX 1024;

class MR_MovementCap
{
   public:

      // Common attributes
      MR_MoveType mMoveType;
      MR_UInt32   mSpeed;              // in mm/sec  .. normal speed of the movement 
      MR_UInt32   mMinimumSpeed;       //
      MR_UInt32   mAcceleration;       // in mm/sec2 .. for turn and transition from one movement to an other
      MR_UInt32   mDeceleration;       // in mm/sec2
      MR_UInt32   mRequiredWidth;
      MR_UInt32   mRequiredHeight;
      MR_UInt32   mMinimumHeight;      // Minimum space between the object position and the ground

      // On surface attribute
      MR_UInt32   mStepHeight;
      MR_UInt32   mMaxStepLen;      
      MR_UInt32   mFallingAcceleration;// Normally 9800mm/sec2 except for flying modes
      MR_UInt32   mRequiredTraction;   // all character that can walk should supply a 
                                       // movement with no traction requirement

      // On air attributes
      MR_UInt32   mMaxVerticalSpeed;
      MR_UInt32   mVerticalAcceleration;

};

class MR_MovementEntry
{
   public:
      BOOL                   mAvaillable;
      const MR_MovementCap*  mMovement;

};

class MR_MovementState
{
   public:
      const MR_MovementCap*  mCurrentMovement;
      MR_3DCoordinate        mPosition;
      MR_Angle               mOrientation;
      MR_UInt32              mCurrentSpeed;   // Horizontal speed
      MR_UInt32              mVerticalSpeed;

};

