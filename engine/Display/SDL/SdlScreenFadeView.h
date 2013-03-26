
// SdlScreenFadeView.h
//
// Copyright (c) 2013 Michael Imamura.
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
	namespace Display {
		class ScreenFade;
	}
}
struct SDL_Surface;

namespace HoverRace {
namespace Display {
namespace SDL {

/**
 * SDL view for ScreenFade.
 * This implementation uses a screen-size RGB surface with a per-surface alpha.
 * @author Michael Imamura
 */
class MR_DllDeclare SdlScreenFadeView : public SdlView<ScreenFade>
{
	typedef SdlView<ScreenFade> SUPER;
	public:
		SdlScreenFadeView(SdlDisplay &disp, ScreenFade &model) :
			SUPER(disp, model), opacityChanged(true), computedAlpha(0) { }
		virtual ~SdlScreenFadeView() { }

	public:
		virtual void OnModelUpdate(int prop);

	public:
		virtual void PrepareRender();
		virtual void Render();

	private:
		void Update();

	private:
		bool opacityChanged;
		MR_UInt8 computedAlpha;
};

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
