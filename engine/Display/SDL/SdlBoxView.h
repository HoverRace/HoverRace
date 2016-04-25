
// SdlBoxView.h
//
// Copyright (c) 2016 Michael Imamura.
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

#include "SdlView.h"

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
		namespace SDL {
			class SdlDisplay;
		}
	}
}

namespace HoverRace {
namespace Display {
namespace SDL {

/**
 * Base class for FillBox views.
 * @author Michael Imamura
 */
template<class T>
class MR_DllDeclare SdlBoxView : public SdlView<T>
{
	using SUPER = SdlView<T>;

public:
	SdlBoxView(SdlDisplay &disp, T &model) :
		SUPER(disp, model), screenPos(0, 0), screenSize(0, 0) { }
	virtual ~SdlBoxView() { }

public:
	Vec2 GetScreenPos() const override { return screenPos; }
	Vec2 GetScreenSize() const override { return screenSize; }

protected:
	/**
	 * Update the screen-space bounds from the UI-space bounds.
	 */
	void CalcScreenBounds()
	{
		const Vec2 &size = this->model.GetSize();
		double w = size.x;
		double h = size.y;

		screenPos = this->display.LayoutUiPosition(
			this->model.GetAlignedPos(w, h));

		if (!this->model.IsLayoutUnscaled()) {
			double uiScale = this->display.GetUiScale();
			w *= uiScale;
			h *= uiScale;
		}
		screenSize.x = w;
		screenSize.y = h;
	}

protected:
	Vec2 screenPos;
	Vec2 screenSize;
};

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
