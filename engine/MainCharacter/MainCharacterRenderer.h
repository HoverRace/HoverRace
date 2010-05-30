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

#ifndef MAIN_CHARACTER_RENDERER_H
#define MAIN_CHARACTER_RENDERER_H

#include "../VideoServices/3DViewPort.h"
#include "../VideoServices/SoundServer.h"
#include "../Util/DllObjectFactory.h"

#ifdef MR_ENGINE
#define MR_DllDeclare   __declspec(dllexport)
#else
#define MR_DllDeclare   __declspec(dllimport)
#endif

class MR_DllDeclare MR_MainCharacterRenderer : public MR_ObjectFromFactory
{
	public:
		// Construction
		MR_MainCharacterRenderer(const MR_ObjectFromFactoryId & pId);
		~MR_MainCharacterRenderer();

		virtual void Render(MR_3DViewPort *pDest, const MR_3DCoordinate &pPosition, MR_Angle pOrientation, BOOL pMotorOn, int pHoverId, int pModel) = 0;

		// Sound list
		virtual HoverRace::VideoServices::ShortSound *GetLineCrossingSound() = 0;
		virtual HoverRace::VideoServices::ShortSound *GetStartSound() = 0;
		virtual HoverRace::VideoServices::ShortSound *GetFinishSound() = 0;
		virtual HoverRace::VideoServices::ShortSound *GetBumpSound() = 0;
		virtual HoverRace::VideoServices::ShortSound *GetJumpSound() = 0;
		virtual HoverRace::VideoServices::ShortSound *GetFireSound() = 0;
		virtual HoverRace::VideoServices::ShortSound *GetMisJumpSound() = 0;
		virtual HoverRace::VideoServices::ShortSound *GetMisFireSound() = 0;
		virtual HoverRace::VideoServices::ShortSound *GetOutOfCtrlSound() = 0;
		virtual HoverRace::VideoServices::ContinuousSound *GetMotorSound() = 0;
		virtual HoverRace::VideoServices::ContinuousSound *GetFrictionSound() = 0;

};

#undef MR_DllDeclare
#endif
