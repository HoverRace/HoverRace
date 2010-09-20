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

#include "StdAfx.h"

#include "OS.h"

#include "Profiler.h"

#ifdef _DEBUG

namespace HoverRace {
namespace Util {

class ProfilerMaster
{
	public:
		OS::timestamp_t mPauseStart;
		OS::timestamp_t mTimeOffset;

		// TimerFunctions
		OS::timestamp_t GetTime();
		OS::timestamp_t PauseTime();						  // Same as GetTime but it also stop the timer
		void UnPauseTime();

		void Reset();
		void PrintStats();
};

static ProfilerSampler *gProfilerSamplerList = NULL;
static ProfilerMaster gProfilerMaster;

void ResetProfilerStats()
{
	gProfilerMaster.Reset();
}

void OutputProfilerStats()
{
	gProfilerMaster.PrintStats();
}

// ProfilerSampler
ProfilerSampler::ProfilerSampler(const char *pName)
{
	mName = pName;
	mNext = gProfilerSamplerList;
	gProfilerSamplerList = this;

	Reset();
}

void ProfilerSampler::Reset()
{
	mNbCall = 0;
	mTotalTime = 0;
	mMinPeriod = 10000000;
	mMaxPeriod = 0;

	mLastPeriodStart = 0;
}

void ProfilerSampler::StartSample()
{
	mLastPeriodStart = gProfilerMaster.GetTime();
}

void ProfilerSampler::EndSample()
{
	int lDuration = gProfilerMaster.PauseTime() - mLastPeriodStart;

	mNbCall++;
	mTotalTime += lDuration;

	if(lDuration < mMinPeriod) {
		mMinPeriod = lDuration;
	}

	if(lDuration > mMaxPeriod) {
		mMaxPeriod = lDuration;
	}

	gProfilerMaster.UnPauseTime();
}

// ProfilerMaster
OS::timestamp_t ProfilerMaster::GetTime()
{
	return timeGetTime() - mTimeOffset;
}

OS::timestamp_t ProfilerMaster::PauseTime()
{
	mPauseStart = GetTime();

	return mPauseStart;
}

void ProfilerMaster::UnPauseTime()
{
	mTimeOffset += GetTime() - mPauseStart;
}

void ProfilerMaster::Reset()
{
	// Reset all the Samplers
	ProfilerSampler *lCurrent = gProfilerSamplerList;

	while(lCurrent != NULL) {
		lCurrent->Reset();
		lCurrent = lCurrent->mNext;
	}
}

void ProfilerMaster::PrintStats()
{
	PauseTime();
	/*
	   TRACE( " Hit    Total  Avg    Min    Max   Name\n" );

	   ProfilerSampler* lCurrent = gProfilerSamplerList;

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

	FILE *lFile = fopen("Stat.out", "a");

	if(lFile != NULL) {
		fprintf(lFile, " Hit    Total  Avg    Min    Max   Name\n");

		ProfilerSampler *lCurrent = gProfilerSamplerList;

		while(lCurrent != NULL) {
			int lAvg = 0;

			if(lCurrent->mNbCall > 0) {
				lAvg = lCurrent->mTotalTime / lCurrent->mNbCall;
			}

			fprintf(lFile, "%-6d %-6d %-6d %-6d %-6d %s\n", lCurrent->mNbCall, lCurrent->mTotalTime, lAvg, lCurrent->mMinPeriod, lCurrent->mMaxPeriod, lCurrent->mName);

			lCurrent = lCurrent->mNext;
		}

		fclose(lFile);
	}

	UnPauseTime();
}

}  // namespace Util
}  // namespace HoverRace

#endif
