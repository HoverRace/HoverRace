
// SdlFillBoxView.h
//
// Copyright (c) 2013, 2014 Michael Imamura.
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

#include "SdlDisplay.h"
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
		class FillBox;
	}
}

namespace HoverRace {
namespace Display {
namespace SDL {

/**
 * SDL view for FillBox.
 * @author Michael Imamura
 */
class MR_DllDeclare SdlFillBoxView : public SdlView<FillBox>
{
	typedef SdlView<FillBox> SUPER;
	public:
		SdlFillBoxView(SdlDisplay &disp, FillBox &model) :
			SUPER(disp, model),
			screenPos(0, 0), screenSize(0, 0) { }
		virtual ~SdlFillBoxView() { }

	public:
		void OnModelUpdate(int) override { }

	public:
		Vec2 GetScreenPos() const override { return screenPos; }
		Vec2 GetScreenSize() const override { return screenSize; }
		Vec3 Measure() override;
		void PrepareRender() override { }
		void Render() override;

	private:
		Vec2 screenPos;
		Vec2 screenSize;
};

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
