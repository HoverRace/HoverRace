// PhysicalCollision.h
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



#ifndef PHYSICAL_COLLISION_H
#define PHYSICAL_COLLISION_H

#include "ContactEffect.h"

#ifdef MR_MODEL
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif



class MR_DllDeclare MR_InertialMoment
{
   public:
      enum { eInfiniteWeight = 1000000 };

      double    mWeight;          // in Kg

      MR_Int32  mXSpeed;
      MR_Int32  mYSpeed;
      MR_Int32  mZSpeed;         // in mm/sec, negative mean down

      // Helper functions
      void ComputeCollision( const MR_InertialMoment*                   pObstacle,
                             MR_Angle                                   pHorizontalDirection );

};

class MR_PhysicalCollision: public MR_ContactEffect, public MR_InertialMoment
{

};




#undef MR_DllDeclare
      
#endif
