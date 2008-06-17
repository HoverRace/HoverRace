// FreeElementBase.h
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
// The class defined in this file can be used as a base class for 
// free elements
//
//

#ifndef FREE_ELEMENT_BASE_H
#define FREE_ELEMENT_BASE_H

#include "../Model/MazeElement.h"
#include "ResourceLib.h"

#ifdef MR_OBJ_FAC_TOOLS
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif


class MR_FreeElementBase:public MR_FreeElement
{
   protected:
      const MR_ResActor* mActor;
      int                mCurrentSequence;
      int                mCurrentFrame;    

   public:
      MR_DllDeclare  MR_FreeElementBase( const MR_ObjectFromFactoryId& pId );
      MR_DllDeclare ~MR_FreeElementBase();
   
      // Rendering stuff
      MR_DllDeclare void Render( MR_3DViewPort* pDest, MR_SimulationTime pTime );

};

#undef MR_DllDeclare

#endif

