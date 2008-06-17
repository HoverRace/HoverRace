// Profiler.cpp
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

#include <Mmsystem.h>


#include "Profiler.h"

#ifdef _DEBUG

class MR_ProfilerMaster
{
   public:
      int mPauseStart;
      int mTimeOffset;

      // TimerFunctions
      int  GetTime();
      int  PauseTime();   // Same as GetTime but it also stop the timer
      void UnPauseTime();

      void Reset();
      void PrintStats();
};


static MR_ProfilerSampler*  gProfilerSamplerList = NULL;
static MR_ProfilerMaster    gProfilerMaster;


void MR_ResetProfilerStats()
{
   gProfilerMaster.Reset();
}

void MR_OutputProfilerStats()
{
   gProfilerMaster.PrintStats();
}


// MR_ProfilerSampler      
MR_ProfilerSampler::MR_ProfilerSampler( const char* pName )
{
   mName = pName;
   mNext = gProfilerSamplerList;
   gProfilerSamplerList = this;

   Reset();
}

void MR_ProfilerSampler::Reset()
{
   mNbCall    = 0;
   mTotalTime = 0;
   mMinPeriod = 10000000;
   mMaxPeriod = 0;

   mLastPeriodStart = 0;
}



void MR_ProfilerSampler::StartSample()
{
   mLastPeriodStart = gProfilerMaster.GetTime();
}

void MR_ProfilerSampler::EndSample()
{
   int lDuration = gProfilerMaster.PauseTime()-mLastPeriodStart;

   mNbCall++;
   mTotalTime += lDuration;

   if( lDuration < mMinPeriod )
   {
      mMinPeriod = lDuration;
   }
   
   if( lDuration > mMaxPeriod )
   {
      mMaxPeriod = lDuration;
   }
   
   gProfilerMaster.UnPauseTime();
}

// MR_ProfilerMaster
int MR_ProfilerMaster::GetTime()
{
   return timeGetTime()-mTimeOffset;
}

int MR_ProfilerMaster::PauseTime()
{
   mPauseStart = GetTime();

   return mPauseStart;
}

void MR_ProfilerMaster::UnPauseTime()
{
   mTimeOffset += GetTime()- mPauseStart;
}

void MR_ProfilerMaster::Reset()
{
   // Reset all the Samplers
   MR_ProfilerSampler* lCurrent = gProfilerSamplerList;

   while( lCurrent != NULL )
   {
      lCurrent->Reset();
      lCurrent = lCurrent->mNext;
   }
}

void MR_ProfilerMaster::PrintStats()
{
   PauseTime();
   /*
   TRACE( " Hit    Total  Avg    Min    Max   Name\n" );

   MR_ProfilerSampler* lCurrent = gProfilerSamplerList;

   while( lCurrent != NULL )
   {
      int lAvg = 0;

      if(lCurrent->mNbCall > 0 )
      {
         lAvg = lCurrent->mTotalTime/lCurrent->mNbCall;
      }

      TRACE
      (
         "%-6d %-6d %-6d %-6d %-6d %s\n",
         lCurrent->mNbCall,
         lCurrent->mTotalTime,
         lAvg,
         lCurrent->mMinPeriod,
         lCurrent->mMaxPeriod,
         lCurrent->mName
      );
      
      lCurrent = lCurrent->mNext;
   }
   */

   FILE* lFile = fopen("Stat.out", "a" );

   if( lFile != NULL )
   {
      fprintf( lFile, " Hit    Total  Avg    Min    Max   Name\n" );

      MR_ProfilerSampler* lCurrent = gProfilerSamplerList;

      while( lCurrent != NULL )
      {
         int lAvg = 0;

         if( lCurrent->mNbCall > 0 ) 
         {
            lAvg = lCurrent->mTotalTime/lCurrent->mNbCall;
         }

         fprintf
         (
            lFile,
            "%-6d %-6d %-6d %-6d %-6d %s\n",
            lCurrent->mNbCall,
            lCurrent->mTotalTime,
            lAvg,
            lCurrent->mMinPeriod,
            lCurrent->mMaxPeriod,
            lCurrent->mName
         );
      
         lCurrent = lCurrent->mNext;
      }

      fclose( lFile );
   }

   UnPauseTime();
}

#endif
