// RaceEffects.h
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

#include "ContactEffect.h"

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
namespace Model {

class SpeedDoubler : public ContactEffect
{
	public:
		// No parameter
};

class FuelGain : public ContactEffect
{
	public:
		double mFuelQty;						  // in seconds per millisecond at the pump
};

class LostOfControl : public ContactEffect
{
	public:
		enum mType { eMissile, eMine };
		int mType;
		int mElementId;							  // Used for mines only
		int mHoverId;							  // Craft that created the effect
};

class CheckPoint : public ContactEffect
{
	public:
		enum CheckPointType { eFinishLine, eCheck1, eCheck2 };
		CheckPointType mType;
};

class PowerUpEffect : public ContactEffect
{
	public:
		int mElementPermId;
};

}  // namespace Model
}  // namespace HoverRace

#undef MR_DllDeclare
