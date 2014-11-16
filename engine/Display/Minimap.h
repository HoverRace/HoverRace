
// Minimap.h
//
// Copyright (c) 2014 Michael Imamura.
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
		class Picture;
		class SymbolIcon;
	}
}

namespace HoverRace {
namespace Display {

/**
 * A miniature map of the track for the HUD.
 * @author Michael Imamura
 */
class MR_DllDeclare Minimap : public HudDecor
{
	typedef HudDecor SUPER;

private:
	Minimap() = delete;
public:
	Minimap(Display &display);
	virtual ~Minimap() { }

protected:
	void OnHudRescaled(const Vec2 &hudScale) override;

protected:
	void FireModelUpdate(int prop) override;
public:
	void Advance(Util::OS::timestamp_t tick) override;

private:
	double rescale;
	Vec2 mapScale;
	std::shared_ptr<Picture> mapPic;
	std::shared_ptr<SymbolIcon> playerIcon;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
