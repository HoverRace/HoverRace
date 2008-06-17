// MissileLine.h

#ifndef MISSILE_H
#define MISSILE_H

#include "../ObjFacTools/FreeElementBase.h"
#include "../Model/RaceEffects.h"
#include "../Model/PhysicalCollision.h"

class MR_Missile:public MR_FreeElementBase, protected MR_CylinderShape
{
   protected:

      // Shape interface
      MR_Int32 ZMin()const;
      MR_Int32 ZMax()const;
      MR_Int32 AxisX()const;
      MR_Int32 AxisY()const;
      MR_Int32 RayLen()const;


   private:

      MR_SimulationTime    mLived;
      MR_PhysicalCollision mCollisionEffect;
      MR_LostOfControl     mLostOfControlEffect;
      MR_ContactEffectList mEffectList;

      double               mXSpeed;
      double               mYSpeed;

      BOOL                 mBounceSoundEvent;
      MR_ShortSound*       mBounceSound;
      MR_ContinuousSound*  mMotorSound;


   public:
      MR_Missile( const MR_ObjectFromFactoryId& pId );      
      ~MR_Missile();


   protected:

      // ContactEffectShapeInterface
      const MR_ContactEffectList* GetEffectList();
      const MR_ShapeInterface*    GetGivingContactEffectShape();
      const MR_ShapeInterface*    GetReceivingContactEffectShape();

      int   Simulate( MR_SimulationTime pTimeSlice, MR_Level* pLevel, int pRoom );
      int   InternalSimulate( MR_SimulationTime pDuration, MR_Level* pLevel, int pRoom );

      void  ApplyEffect( const MR_ContactEffect* pEffect,  MR_SimulationTime pTime, MR_SimulationTime pDuration, BOOL pValidDirection, MR_Angle pHorizontalDirection, MR_Int32 pZMin, MR_Int32 pZMax, MR_Level* pLevel );

      // Network state
      MR_ElementNetState GetNetState()const;
      void               SetNetState( int pDataLen, const MR_UInt8* pData );

      // Sounds 
      void MR_Missile::PlayExternalSounds( int pDB, int pPan );
};

#endif

