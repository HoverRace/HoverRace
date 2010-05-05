// ContactEffect.h
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

#include <vector>

#include "../Model/Shapes.h"

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

class MR_DllDeclare MR_ContactEffect
{
	// Base class for all the contact effects
	// you must use typeinfo to determine the real type of the effect
	public:
		virtual ~ MR_ContactEffect();

												  // If the effect is unknow, use that one instead
		virtual const MR_ContactEffect *GetAlternate() const;
		// Usually return NULL
};
typedef std::vector<MR_ContactEffect*> MR_ContactEffectList;

#undef MR_DllDeclare
