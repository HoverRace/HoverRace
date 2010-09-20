// Profiler.h
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

#pragma once

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

#ifdef _DEBUG

namespace HoverRace {
namespace Util {

// Required declaration
class MR_DllDeclare ProfilerSampler
{
	// Theses objects must always be declared staticly
	friend class ProfilerMaster;

	private:
		ProfilerSampler *mNext;
		const char *mName;						  // Name or comment, used during outputs
		int mNbCall;
		int mTotalTime;
		int mMinPeriod;
		int mMaxPeriod;

		int mLastPeriodStart;

		void Reset();

	public:
		ProfilerSampler(const char *pName);

		void StartSample();
		void EndSample();

};

class ContextSampler
{
	private:
		ProfilerSampler *mSampler;

	public:
		ContextSampler(ProfilerSampler * pSampler) {
			mSampler = pSampler;
			mSampler->StartSample();
		}
		~ContextSampler() {
			mSampler->EndSample();
		}

};

// Macros declaration

#define MR_SAMPLE_START( pId, pName ) \
static HoverRace::Util::ProfilerSampler pId( pName ); \
pId.StartSample();

#define MR_SAMPLE_END( pId ) \
pId.EndSample();

#define MR_SAMPLE_CONTEXT( pName ) \
static HoverRace::Util::ProfilerSampler TheSampler( pName ); \
HoverRace::Util::ContextSampler TheContextSampler( &TheSampler );

#define MR_PRINT_STATS( pPeriod ) \
{ \
	static time_t lLastRefresh = time( NULL ); \
	if( time( NULL ) >= lLastRefresh+pPeriod ) \
	{ \
		HoverRace::Util::OutputProfilerStats(); \
		HoverRace::Util::ResetProfilerStats(); \
		lLastRefresh = time( NULL ); \
	} \
}

// Other useful functions
void MR_DllDeclare ResetProfilerStats();
void MR_DllDeclare OutputProfilerStats();

}  // namespace Util
}  // namespace HoverRace

#else

// Dummy macro definitions
#define MR_SAMPLE_START( pId, pName )

#define MR_SAMPLE_END( pId )

#define MR_SAMPLE_CONTEXT( pName )

#define MR_PRINT_STATS( pPeriod )
#endif

#undef MR_DllDeclare
