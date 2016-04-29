
// Picture.h
//
// Copyright (c) 2014, 2016 Michael Imamura.
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

#include "Res.h"

#include "Box.h"

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
 * Draws a single texture.
 * @author Michael Imamura
 */
class MR_DllDeclare Picture : public Box
{
	using SUPER = Box;

public:
	struct Props
	{
		enum
		{
			TEXTURE = SUPER::Props::NEXT_,
			COLOR,
			NEXT_,  ///< First index for subclasses.
		};
	};

public:
	Picture(std::shared_ptr<Res<Texture>> texture,
		const Vec2 &size, const Color color = COLOR_WHITE,
		uiLayoutFlags_t layoutFlags = 0);
	Picture(std::shared_ptr<Res<Texture>> texture,
		double w, double h, const Color color = COLOR_WHITE,
		uiLayoutFlags_t layoutFlags = 0);
	virtual ~Picture() { }

public:
	virtual void AttachView(Display &disp) { AttachViewDynamic(disp, this); }

public:

	std::shared_ptr<Res<Texture>> ShareTexture() const { return texture; }
	void SetTexture(std::shared_ptr<Res<Texture>> texture);

	const Color GetColor() const { return color; }
	void SetColor(const Color color);

private:
	std::shared_ptr<Res<Texture>> texture;
	Color color;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
