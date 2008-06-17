// FuzzyLogic.cpp
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

#include "stdafx.h"
#include "FuzzyLogic.h"
// Local variables

#define MR_RAND_TABLE_SIZE  4096

static int gRandTable[ MR_RAND_TABLE_SIZE ];
static int gRandIndex;

void MR_InitFuzzyModule()
{
   for( int lCounter = 0; lCounter < MR_RAND_TABLE_SIZE; lCounter++ )
   {
      gRandTable[ lCounter ] = rand();
   }
   gRandIndex = 0;
}

int MR_Rand()
{
   return gRandTable[ (gRandIndex++)&(MR_RAND_TABLE_SIZE-1) ];
}



MR_ProbTable::MR_ProbTable()
{
   mNbProb = 0;
   mTotalProb = 0;
}

void MR_ProbTable::Clear()
{
   mNbProb = 0;
   mTotalProb = 0;
}

void MR_ProbTable::AddProb( int pWeight )
{
   if( mNbProb < NB_PROB_MAX )
   {
      mTotalProb += pWeight;
      mProb[ mNbProb++ ] = mTotalProb;
   }
   else
   {
      ASSERT( FALSE );
   }
}

int MR_ProbTable::GetVal()
{
   int lRand = MR_Rand()%mTotalProb;
   int lReturnValue = 0;

   for( ;lReturnValue < mNbProb-1; lReturnValue++ )
   {
      if( lRand < mProb[ lReturnValue ] )
      {
         break;
      }
   }
   return lReturnValue;
}
