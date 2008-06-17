// FreeElementMovingInterface.h
//
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


#ifndef FREE_ELEMENT_MOVING_HELPER_H
#define FREE_ELEMENT_MOVING_HELPER_H

// Includes

#include "Level.h"

#ifdef MR_MODEL
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif


// Helper class
class MR_ObstacleCollisionReport
{
   private:
      BOOL     mInMaze;               // If false, forget the rest
    
      int      mCurrentRoom;

      MR_Int32 mShapeTop;             // Absolute
      MR_Int32 mShapeBottom;          // Absolute

      MR_Int32 mClosestFloor;         // Relative to Bottom
      MR_Int32 mClosestCeiling;       // Relative to Top

      BOOL     mHaveObstacleContact;  
      MR_Int32 mObstacleTop;          // Absolute
      MR_Int32 mObstacleBottom;       // Absolute

      BOOL     AlmostCompleted()const;

      void     GetContactWithFeaturesAndActors( MR_Level*                   pLevel,
                                                const MR_ShapeInterface*    pShape,
                                                int                         pRoom,
                                                MR_FreeElement*             pElement,
                                                BOOL                        pIgnoreActors );         

   public:

      BOOL     MR_DllDeclare IsInMaze()const;          // Call notting else if false
      BOOL     MR_DllDeclare HaveContact()const;
      int      MR_DllDeclare Room()const;              // Room containing the shape
      MR_Int32 MR_DllDeclare StepHeight()const;        // Negative Height of the floor if Not having contact, Shape height if higher than shape
      MR_Int32 MR_DllDeclare CeilingStepHeight()const; // Negative Ceiling Height if Not having contact, Shape depth if lower than shape
      MR_Int32 MR_DllDeclare SpaceToFloor()const;      // Negative if in floor
      MR_Int32 MR_DllDeclare SpaceToCeiling()const;    // Negative if in ceiling

      MR_Int32 MR_DllDeclare LargestHoleHeight()const;
      MR_Int32 MR_DllDeclare LargestHoleStep()const;


      void MR_DllDeclare GetContactWithObstacles( MR_Level*                   pLevel,
                                                  const MR_ShapeInterface*    pShape,
                                                  int                         pRoom,
                                                  MR_FreeElement*             pElement,
                                                  BOOL                        pIgnoreActors = FALSE );


};

// Helper class that determine if a move can be made
// If not the function 

#undef MR_DllDeclare

#endif
