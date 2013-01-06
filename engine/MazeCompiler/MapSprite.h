// MapSprite.h
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
#include "../VideoServices/Sprite.h"

#ifdef _WIN32
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

class MR_DllDeclare MapSprite : public VideoServices::Sprite
{
	typedef VideoServices::Sprite SUPER;
	protected:
		int mXMin;
		int mXMax;
		int mYMin;
		int mYMax;

		void ComputeMinMax(Model::Level *pLevel);
		void DrawMap(Model::Level *pLevel);

	public:

		BOOL CreateMap(Model::Level *pLevel, int &pX0, int &pY0, int &pX1, int &pY1);

};

}  // namespace MazeCompiler
}  // namespace HoverRace
