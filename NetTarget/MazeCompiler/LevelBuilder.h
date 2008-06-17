// LevelBuilder.h
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

#ifndef LEVELBUILDER_H
#define LEVELBUILDER_H

#include "../Model/Level.h"


class MR_VisibleStep;

class MR_LevelBuilder: public MR_Level
{
   protected:

      // Implementation helpers
      static double ComputeShapeConst( Section* pSection );

      // Main methods

      BOOL Parse( FILE* pFile );
      BOOL ComputeVisibleZones();
      BOOL ComputeAudibleZones();

      void OrderVisibleSurfaces();

   private:
      void TestForVisibility(  MR_VisibleStep     *pPreviousStep,
                               int                *pDestArray,
                               int                &pDestIndex,
                               int                 pNewLeftNodeIndex );

      static int OrderFloor( const void* pSurface0, const void* pSurface1 );
      static int OrderCeiling( const void* pSurface0, const void* pSurface1 );


   public:

      BOOL InitFromFile( FILE* pFile );

};


#endif

