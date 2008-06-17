// BumperGate.h
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

#ifndef BUMPERGATE_H
#define BUMPERGATE_H

#include "../ObjFacTools/FreeElementBase.h"
#include "../Model/RaceEffects.h"
#include "../Model/PhysicalCollision.h"

class MR_BumperGate:public MR_FreeElementBase, protected MR_CylinderShape
{
   protected:

      // Shape interface
      MR_Int32 ZMin()const;
      MR_Int32 ZMax()const;
      MR_Int32 AxisX()const;
      MR_Int32 AxisY()const;
      MR_Int32 RayLen()const;


   private:

      MR_SimulationTime    mTimeSinceLastCollision;
      int                  mLastState;
      MR_PhysicalCollision mCollisionEffect;
      MR_ContactEffectList mEffectList;

   public:
      MR_BumperGate( const MR_ObjectFromFactoryId& pId );      
      ~MR_BumperGate();


   protected:

      // ContactEffectShapeInterface
      const MR_ContactEffectList* GetEffectList();
      const MR_ShapeInterface*    GetGivingContactEffectShape();
      const MR_ShapeInterface*    GetReceivingContactEffectShape();

      int   Simulate( MR_SimulationTime pTimeSlice, MR_Level* pLevel, int pRoom );

      void  ApplyEffect( const MR_ContactEffect* pEffect,  MR_SimulationTime pTime, MR_SimulationTime pDuration, BOOL pValidDirection, MR_Angle pHorizontalDirection, MR_Int32 pZMin, MR_Int32 pZMax, MR_Level* pLevel );

};

#endif

