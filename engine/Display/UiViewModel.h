
// UiViewModel.h
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

#include "Vec.h"
#include "ViewModel.h"

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

/**
 * Base class for UI (2D) components.
 * @author Michael Imamura
 */
class MR_DllDeclare UiViewModel : public ViewModel
{
	typedef ViewModel SUPER;

	public:
		struct Props
		{
			enum {
				POS,
				NEXT_,  ///< First index for subclasses.
			};
		};

	public:
		UiViewModel() : SUPER(), pos(0, 0) { }
		virtual ~UiViewModel() { }

	public:
		/**
		 * Get the position of the component.
		 * @return The position, relative to the parent (if any).
		 */
		const Vec2 &GetPos() const { return pos; }
		void SetPos(const Vec2 &pos);
		/// Convenience function for SetPos(const Vec2&).
		void SetPos(double x, double y) { SetPos(Vec2(x, y)); }

	private:
		Vec2 pos;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
