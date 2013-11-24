
// Hud.h
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

#include "../Util/OS.h"
#include "HudDecor.h"

#include "Container.h"

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
		class Display;
	}
	namespace MainCharacter {
		class MainCharacter;
	}
}

namespace HoverRace {
namespace Display {

/**
 * The container for the heads-up display.
 * @author Michael Imamura
 */
class MR_DllDeclare Hud : public Container
{
	typedef Container SUPER;

	public:
		struct Props
		{
			enum {
				PLAYER = SUPER::Props::NEXT_,
				VISIBLE,
				NEXT_,  ///< First index for subclasses.
			};
		};
		struct HudAlignment
		{
			enum type {
				ABOVE,  ///< Centered in the top-half of the screen.
				BELOW,  ///< Centered in the bottom-half of the screen.
				N,      ///< Center-north, stacked left-to-right.
				NNE,    ///< Northeast corner, stacked right-to-left.
				NE,     ///< Northeast corner, only top is visible.
				ENE,    ///< Northeast corner, stacked top-to-bottom.
				E,      ///< Center-east, stacked top-to-bottom.
				ESE,    ///< Southeast corner, stacked bottom-to-top.
				SE,     ///< Southeast corner, only top is visible.
				SSE,    ///< Southeast corner, stacked right-to-left.
				S,      ///< Center-south, stacked left-to-right.
				SSW,    ///< Southwest corner, stacked left-to-right.
				SW,     ///< Southwest corner, only top is visible.
				WSW,    ///< Southwest corner, stacked bottom-to-top.
				W,      ///< Center-west, stacked top-to-bottom.
				WNW,    ///< Northwest corner, stacked top-to-bottom.
				NW,     ///< Northwest corner, only top is visible.
				NNW,    ///< Northwest corner, stacked left-to-right.
			};
			static const size_t NUM = NNW + 1;

			static bool IsCorner(type t) {
				return t == NW || t == NE || t == SE || t == SW;
			}

			static Alignment::alignment_t AlignmentFor(type t) {
				switch (t) {
					case ABOVE:
						return Alignment::S;
					case BELOW:
						return Alignment::N;
					case N:
						return Alignment::N;
					case NNE:
					case NE:
					case ENE:
						return Alignment::NE;
					case E:
						return Alignment::E;
					case ESE:
					case SE:
					case SSE:
						return Alignment::SE;
					case S:
						return Alignment::S;
					case SSW:
					case SW:
					case WSW:
						return Alignment::SW;
					case W:
						return Alignment::W;
					case WNW:
					case NW:
					case NNW:
					default:
						return Alignment::NW;
				}
			}
		};

	public:
		Hud(Display &display, MainCharacter::MainCharacter *player,
			const Vec2 &size, bool clip=true,
			uiLayoutFlags_t layoutFlags=0);
		virtual ~Hud() { }

	/* Using the view for Container.
	public:
		virtual void AttachView(Display &disp) { AttachViewDynamic(disp, this); }
	*/

	public:
		/**
		 * Append a child element to the end of the list.
		 * @param child The child element; must be a subclass of UiViewModel.
		 * @return The child element, wrapped in a @c std::shared_ptr.
		 */
		template<typename T>
		typename std::enable_if<std::is_base_of<HudDecor, T>::value, std::shared_ptr<T>>::type
		AddHudChild(HudAlignment::type alignment, T *child)
		{
			std::shared_ptr<T> sharedChild = AddChild(child);
			sharedChild->SetPlayer(player);

			// For corner elems, replace the elem instead of adding.
			if (HudAlignment::IsCorner(alignment)) {
				hudChildren[alignment].clear();
			}

			sharedChild->SetAlignment(HudAlignment::AlignmentFor(alignment));

			hudChildren[alignment].emplace_back(sharedChild);

			RequestLayout();
			return sharedChild;
		}

	protected:
		template<typename Fn>
		void ForEachHudChild(Fn fn)
		{
			BOOST_FOREACH(auto &children, hudChildren) {
				BOOST_FOREACH(auto &child, children) {
					fn(child);
				}
			}
		}

	public:
		/**
		 * Retrieve the player being targeted by this HUD.
		 * @return The player (may be @c nullptr).
		 */
		MainCharacter::MainCharacter *GetPlayer() const { return player; }
		void SetPlayer(MainCharacter::MainCharacter *player);

		/**
		 * Check if the HUD is currently visible;
		 * @return @c true if the HUD is visible, @c false otherwise.
		 */
		bool IsVisible() const { return visible; }
		void SetVisible(bool visible);

	private:
		void LayoutCorner(HudAlignment::type alignCorner,
			HudAlignment::type alignH, HudAlignment::type alignV,
			double startX, double startY,
			double scaleX, double scaleY);
	protected:
		virtual void Layout();

	public:
		void Advance(Util::OS::timestamp_t tick);

	private:
		MainCharacter::MainCharacter *player;
		bool visible;
		typedef std::vector<std::shared_ptr<HudDecor>> hudChildList_t;
		std::array<hudChildList_t, HudAlignment::NUM> hudChildren;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
