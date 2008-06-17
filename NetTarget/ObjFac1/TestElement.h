// TestElement.h
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


//
// This element is used to test and build new FreeElements
// 


#ifndef TEST_ELEMENT_H
#define TEST_ELEMENT_H

#include "../ObjFacTools/FreeElementBase.h"
#include "../Model/PhysicalCollision.h"


class MR_TestElement: public MR_FreeElementBase
{
      class Cylinder:public MR_CylinderShape
      {
         public:

            MR_Int32        mRay;
            MR_3DCoordinate mPosition;

            MR_Int32 ZMin()const;
            MR_Int32 ZMax()const;
            MR_Int32 AxisX()const;
            MR_Int32 AxisY()const;
            MR_Int32 RayLen()const;
      };

   
   private:
      MR_SimulationTime mElapsedFrameTime;

      // Logic part (Always present.. who cares it is a test
      int  mXSpeed;
      int  mYSpeed;

      Cylinder      mCollisionShape;                 
      Cylinder      mContactShape;

      MR_PhysicalCollision mContactEffect;
      MR_ContactEffectList mContactEffectList;

   public:
      MR_TestElement( const MR_ObjectFromFactoryId& pId, int pActorRes );
      ~MR_TestElement();  
      
      int Simulate( MR_SimulationTime pDuration, MR_Level* pLevel, int pRoom );


   protected:


      // ContactEffectShapeInterface
      void ApplyEffect( const MR_ContactEffect* pEffect,  MR_SimulationTime pTime, MR_SimulationTime pDuration, BOOL pValidDirection, MR_Angle pHorizontalDirection, MR_Int32 pZMin, MR_Int32 pZMax, MR_Level* pLevel );

      const MR_ContactEffectList* GetEffectList();

      const MR_ShapeInterface* GetObstacleShape( );
      const MR_ShapeInterface* GetReceivingContactEffectShape();
      const MR_ShapeInterface* GetGivingContactEffectShape();

};

#endif

