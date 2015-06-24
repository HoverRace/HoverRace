// LevelBuilder.h
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

#include "../Model/Level.h"
#include "../Util/OS.h"
#include "TrackCompilationLog.h"

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
namespace MazeCompiler {

class VisibleStep;

class MR_DllDeclare LevelBuilder : public Model::Level
{
	using SUPER = Model::Level;

public:
	LevelBuilder(Model::Track &track, const TrackCompilationLogPtr &log);
	virtual ~LevelBuilder() {}

protected:
	static double ComputeShapeConst(Section * pSection);

	bool Parse(std::istream &in);
	bool ComputeVisibleZones();
	bool ComputeAudibleZones();

	void OrderVisibleSurfaces();

private:
	void TestForVisibility(VisibleStep *pPreviousStep, int *pDestArray,
		int &pDestIndex, int pNewLeftNodeIndex);

	static int OrderFloor(const void *pSurface0, const void *pSurface1);
	static int OrderCeiling(const void *pSurface0, const void *pSurface1);

public:
	bool InitFromFile(const Util::OS::path_t &filename);
	bool InitFromStream(std::istream &in);

private:
	TrackCompilationLogPtr log;
};

}  // namespace MazeCompiler
}  // namespace HoverRace

#undef MR_DllDeclare
