
// FuzzyLogic.h
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

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

void MR_DllDeclare MR_InitFuzzyModule();
int MR_DllDeclare MR_Rand();

/// Class created for very fast fuzzzy logic
class MR_DllDeclare MR_ProbTable
{
private:
	static const int NB_PROB_MAX = 16;
	int mNbProb;
	int mTotalProb;
	int mProb[NB_PROB_MAX];

public:
	MR_ProbTable();
	void Clear();

	void AddProb(int pWeight);
	int GetVal();
};

#undef MR_DllDeclare
