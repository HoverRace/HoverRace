
// UiViewModel.h
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

#include "../Util/MR_Types.h"
#include "../Vec.h"
#include "UiLayoutFlags.h"
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
	namespace Control {
		namespace Mouse {
			struct Click;
		}
	}
}

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
				ALIGNMENT,
				NEXT_,  ///< First index for subclasses.
			};
		};

		/**
		 * Imagine the component pinned to the container with a thumbtack.
		 * The thumbtack's position is at GetPos(), and the alignment determines
		 * which corner the component hangs from.
		 */
		struct Alignment
		{
			enum type {
				NW,  ///< Northwest corner (default).
				N,  ///< North (horizontally-centered).
				NE,  ///< Northeast corner.
				E,  ///< East (vertically-centered).
				SE,  ///< Southeast corner.
				S,  ///< South (horizontally-centered).
				SW,  ///< Southwest corner.
				W,  ///< West (vertically-centered).
				CENTER,  ///< Horizontally @e and vertically centered.
			};
		};

	public:
		UiViewModel(uiLayoutFlags_t layoutFlags=0) :
			SUPER(), pos(0, 0), alignment(Alignment::NW),
			layoutFlags(layoutFlags) { }
		virtual ~UiViewModel() { }

	public:
		virtual bool OnMouseMoved(const Vec2 &pos) { return false; }
		virtual bool OnMousePressed(const Control::Mouse::Click &click) { return false; }
		virtual bool OnMouseReleased(const Control::Mouse::Click &click) { return false; }

	public:
		/**
		 * Get the position of the component.
		 * @return The position, relative to the parent (if any).
		 * @see GetAlignment()
		 */
		const Vec2 &GetPos() const { return pos; }
		void SetPos(const Vec2 &pos);
		/// Convenience function for SetPos(const Vec2&).
		void SetPos(double x, double y) { SetPos(Vec2(x, y)); }

		/**
		 * Retrieve the alignment of the component.
		 * @return The alignment.
		 * @see UiViewModel::Alignment
		 */
		const Alignment::type GetAlignment() const { return alignment; }
		void SetAlignment(Alignment::type alignment);

		Vec2 GetAlignedPos(const Vec2 &pos, double w, double h) const;

		/**
		 * Retrieve the position adjusted by the current alignment.
		 * @param w The width of the component.
		 * @param h The height of the component.
		 * @return The adjusted position.
		 */
		Vec2 GetAlignedPos(double w, double h) const
		{
			return GetAlignedPos(pos, w, h);
		}

		/**
		 * Retrieve the layout flags.
		 * @see UiViewModel::LayoutFlags
		 */
		uiLayoutFlags_t GetLayoutFlags() const { return layoutFlags; }

		// Convenience functions for querying layout flags.
		uiLayoutFlags_t IsLayoutUnscaled() const { return layoutFlags & UiLayoutFlags::UNSCALED; }
		uiLayoutFlags_t IsLayoutFloating() const { return layoutFlags & UiLayoutFlags::FLOATING; }

	private:
		Vec2 pos;
		Alignment::type alignment;
		uiLayoutFlags_t layoutFlags;
};
typedef std::shared_ptr<UiViewModel> UiViewModelPtr;

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
