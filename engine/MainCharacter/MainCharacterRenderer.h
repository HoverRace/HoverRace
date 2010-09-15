// MainCharacterRenderer.h
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

#include "../VideoServices/Viewport3D.h"
#include "../VideoServices/SoundServer.h"
#include "../Util/DllObjectFactory.h"

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
namespace MainCharacter {

class MR_DllDeclare MainCharacterRenderer : public Util::ObjectFromFactory
{
	public:
		// Construction
		MainCharacterRenderer(const Util::ObjectFromFactoryId & pId);
		~MainCharacterRenderer();

		virtual void Render(VideoServices::Viewport3D *pDest, const MR_3DCoordinate &pPosition, MR_Angle pOrientation, BOOL pMotorOn, int pHoverId, int pModel) = 0;

		// Sound list
		virtual VideoServices::ShortSound *GetLineCrossingSound() = 0;
		virtual VideoServices::ShortSound *GetStartSound() = 0;
		virtual VideoServices::ShortSound *GetFinishSound() = 0;
		virtual VideoServices::ShortSound *GetBumpSound() = 0;
		virtual VideoServices::ShortSound *GetJumpSound() = 0;
		virtual VideoServices::ShortSound *GetFireSound() = 0;
		virtual VideoServices::ShortSound *GetMisJumpSound() = 0;
		virtual VideoServices::ShortSound *GetMisFireSound() = 0;
		virtual VideoServices::ShortSound *GetOutOfCtrlSound() = 0;
		virtual VideoServices::ContinuousSound *GetMotorSound() = 0;
		virtual VideoServices::ContinuousSound *GetFrictionSound() = 0;

};

}  // namespace MainCharacter
}  // namespace HoverRace

#undef MR_DllDeclare
