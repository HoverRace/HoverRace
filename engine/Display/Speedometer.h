
// Speedometer.h
//
// Copyright (c) 2013-2015 Michael Imamura.
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

#pragma once

#include "HudDecor.h"

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
	namespace Display {
		class Display;
		class FillBox;
	}
}

namespace HoverRace {
namespace Display {

/**
 * HUD velocity indicator.
 * @author Michael Imamura
 */
class MR_DllDeclare Speedometer : public HudDecor
{
	using SUPER = HudDecor;

public:
	Speedometer(Display &display);
	virtual ~Speedometer() { }

protected:
	void OnHudRescaled(const Vec2 &hudScale) override;

public:
	void Advance(Util::OS::timestamp_t tick) override;

private:
	Vec2 gaugeSize;

	// Absolute speed.
	std::shared_ptr<FillBox> absBg;
	std::shared_ptr<FillBox> absFg;
	// Directional speed.
	std::shared_ptr<FillBox> dirBg;
	std::shared_ptr<FillBox> dirFg;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
