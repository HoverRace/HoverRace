
// Wallpaper.h
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

#include "Color.h"
#include "Res.h"

#include "Background.h"

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
		class Texture;
	}
}

namespace HoverRace {
namespace Display {

/**
 * Paints a texture across the entire screen.
 * @author Michael Imamura
 */
class MR_DllDeclare Wallpaper : public Background
{
	typedef Background SUPER;

	public:
		struct Props
		{
			enum
			{
				FILL = SUPER::Props::NEXT_,
				COLOR,
				NEXT_,  ///< First index for subclasses.
			};
		};

		enum class Fill
		{
			ZOOM,  ///< Zoom to fill, maintaining the original aspect ratio.
			STRETCH,  ///< Stretch to fit, ignoring the original aspect ratio.
		};

	public:
		Wallpaper(std::shared_ptr<Res<Texture>> texture,
			Fill fill = Fill::ZOOM, double opacity = 1.0,
			const Color color = COLOR_WHITE);
		virtual ~Wallpaper();

	public:
		virtual void AttachView(Display &disp) { AttachViewDynamic(disp, this); }

	public:
		Fill GetFill() const { return fill; }
		void SetFill(Fill fill);

		const Color GetColor() const { return color; }
		void SetColor(const Color color);

		std::shared_ptr<Res<Texture>> GetTexture() const { return texture; }

	private:
		Fill fill;
		std::shared_ptr<Res<Texture>> texture;
		Color color;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
