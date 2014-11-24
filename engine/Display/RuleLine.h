
// RuleLine.h
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

#include "UiViewModel.h"

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
	}
}

namespace HoverRace {
namespace Display {

/**
 * A horizontal or vertical line.
 *
 * Unlike a FillBox, this line is guaranteed to be at least one pixel in width. 
 *
 * @author Michael Imamura
 */
class MR_DllDeclare RuleLine : public UiViewModel
{
	using SUPER = UiViewModel;

public:
	struct Props
	{
		enum
		{
			DIRECTION = SUPER::Props::NEXT_,
			SIZE,
			COLOR,
			NEXT_,  ///< First index for subclasses.
		};
	};

	enum class Direction { H, V };

public:
	RuleLine(Direction direction, double length, double width,
		const Color color, uiLayoutFlags_t layoutFlags = 0);
	virtual ~RuleLine() { }

public:
	virtual void AttachView(Display &disp) { AttachViewDynamic(disp, this); }

public:
	Direction GetDirection() const { return direction; }
	void SetDirection(Direction direction);

	double GetLength() const { return length; }
	void SetLength(double length);

	double GetWidth() const { return width; }
	void SetWidth(double width);

	const Color GetColor() const { return color; }
	void SetColor(const Color color);

public:
	Vec3 Measure() override;

private:
	Direction direction;
	double length;
	double width;
	Color color;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
