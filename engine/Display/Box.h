
// Box.h
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
 * Base class for low-level bounded widgets.
 * @author Michael Imamura
 */
class MR_DllDeclare Box : public UiViewModel
{
	using SUPER = UiViewModel;

public:
	struct Props
	{
		enum
		{
			SIZE = SUPER::Props::NEXT_,
			NEXT_,  ///< First index for subclasses.
		};
	};

public:
	Box(const Vec2 &size, uiLayoutFlags_t layoutFlags = 0) :
		SUPER(layoutFlags), size(size) { }
	virtual ~Box() { }

public:
	/**
	 * Retrieve the size of the box.
	 * @return The size, where @c x is the width and @c y is the height.
	 */
	const Vec2 &GetSize() const { return size; }
	void SetSize(const Vec2 &size);
	/// Convenience function for SetSize(const Vec2&).
	void SetSize(double w, double h) { SetSize(Vec2(w, h)); }

public:
	virtual void AdjustHeight(double h);
	virtual void AdjustWidth(double w);

public:
	virtual Vec3 Measure() { return size.Promote(); }

private:
	Vec2 size;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
