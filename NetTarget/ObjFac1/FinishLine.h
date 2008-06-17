// FinishLine.h
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

#ifndef FINISH_LINE_H
#define FINISH_LINE_H

#include "../Model/MazeElement.h"
#include "../Model/RaceEffects.h"

// Note.. finish lines are not finish lines..they are check points

class MR_FinishLine:public MR_FreeElement, protected MR_CylinderShape
{
   protected:

      // Shape interface
      MR_Int32 ZMin()const;
      MR_Int32 ZMax()const;
      MR_Int32 AxisX()const;
      MR_Int32 AxisY()const;
      MR_Int32 RayLen()const;


   private:

      MR_CheckPoint        mEffect;
      MR_ContactEffectList mContactEffectList;

   public:
      MR_FinishLine( const MR_ObjectFromFactoryId& pId, MR_CheckPoint::CheckPointType pType );      
      ~MR_FinishLine();


   protected:

      // ContactEffectShapeInterface
      const MR_ContactEffectList* GetEffectList();
      const MR_ShapeInterface* GetReceivingContactEffectShape();

};

#endif

