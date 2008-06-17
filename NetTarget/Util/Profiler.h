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

#ifndef PROFILER_H
#define PROFILER_H

// Includes


#ifdef MR_UTIL
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif

#ifdef _DEBUG

// Required declaration
class MR_DllDeclare MR_ProfilerSampler
{
   // Theses objects must always be declared staticly
   friend class MR_ProfilerMaster;

   private:
      MR_ProfilerSampler* mNext;
      const char*         mName;  // Name or comment, used during outputs
      int                 mNbCall;
      int                 mTotalTime;
      int                 mMinPeriod;
      int                 mMaxPeriod;

      int                 mLastPeriodStart;

      void Reset();

   public:
      MR_ProfilerSampler( const char* pName );

      void StartSample();
      void EndSample();


};

class MR_ContextSampler
{
   private:
      MR_ProfilerSampler* mSampler;

   public:
      MR_ContextSampler( MR_ProfilerSampler* pSampler ){ mSampler = pSampler; mSampler->StartSample(); }
      ~MR_ContextSampler(){ mSampler->EndSample(); }

};

// Macros declaration


#define MR_SAMPLE_START( pId, pName )      \
   static MR_ProfilerSampler pId( pName ); \
   pId.StartSample();

#define MR_SAMPLE_END( pId )      \
   pId.EndSample();

#define MR_SAMPLE_CONTEXT( pName )                   \
   static MR_ProfilerSampler TheSampler( pName );    \
   MR_ContextSampler TheContextSampler( &TheSampler );


#define MR_PRINT_STATS( pPeriod )                \
   {                                             \
      static time_t lLastRefresh = time( NULL ); \
                                                 \
      if( time( NULL ) >= lLastRefresh+pPeriod ) \
      {                                          \
         MR_OutputProfilerStats();               \
         MR_ResetProfilerStats();                \
         lLastRefresh = time( NULL );            \
      }                                          \
   }                                             \



// Other usefull function
void MR_DllDeclare MR_ResetProfilerStats();
void MR_DllDeclare MR_OutputProfilerStats();

#else

// Dummy macro definitions
#define MR_SAMPLE_START( pId, pName )

#define MR_SAMPLE_END( pId )

#define MR_SAMPLE_CONTEXT( pName )


#define MR_PRINT_STATS( pPeriod )

#endif

#undef MR_DllDeclare

#endif
