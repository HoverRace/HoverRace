
// Display.h
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

#include "../Vec.h"
#include "UiFont.h"
#include "Color.h"
#include "UiLayoutFlags.h"
#include "ViewAttacher.h"

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
		class Button;
		class ClickRegion;
		class Container;
		class FillBox;
		class Label;
		class Picture;
		class RuleLine;
		class ScreenFade;
		class SymbolIcon;
		class Wallpaper;
	}
	namespace VideoServices {
		class VideoBuffer;
	}
}

namespace HoverRace {
namespace Display {

/**
 * Base class for display managers.
 *
 * A display manager handles the API-specific tasks of lifecycle stages,
 * instantiating views, and accessing shared resources.
 *
 * @see SdlDisplay
 * @author Michael Imamura
 */
class MR_DllDeclare Display :
	public ViewAttacher<Button>,
	public ViewAttacher<ClickRegion>,
	public ViewAttacher<Container>,
	public ViewAttacher<FillBox>,
	public ViewAttacher<Label>,
	public ViewAttacher<Picture>,
	public ViewAttacher<RuleLine>,
	public ViewAttacher<ScreenFade>,
	public ViewAttacher<SymbolIcon>,
	public ViewAttacher<Wallpaper>
{
	public:
		Display() : uiOrigin(0, 0), uiLayoutFlags(0), uiScale(1.0),
			uiOffset(0, 0), uiScreenSize(1280, 720) { }
		virtual ~Display() { }

	public:
		/**
		 * Retrieve the legacy (8-bit) framebuffer.
		 *
		 * The legacy framebuffer provides an 8-bit surface for drawing.
		 * This framebuffer is drawn as the background layer since it doesn't
		 * have an alpha channel.
		 *
		 * @return The framebuffer.
		 */
		virtual VideoServices::VideoBuffer &GetLegacyDisplay() const = 0;

		/**
		 * Reconfigure the display when the desktop resolution has changed.
		 *
		 * This should be called whenever the desktop resolution changes, so
		 * the aspect ratio can be maintained in windowed mode.
		 *
		 * @param width The width of the desktop in pixels.
		 * @param height The height of the desktop in pixels.
		 */
		virtual void OnDesktopModeChanged(int width, int height) = 0;

		/**
		 * Reconfigure the display based on the new configuration.
		 *
		 * This should be called whenever the video configuration of
		 * Util::Config changes.
		 *
		 * @warning Depending on the underlying API and what configuration
		 * options changed, this may trigger resources to be reloaded or
		 * regenerated.
		 * @note Subclasses should call the base class implementation to
		 *       ensure that the UI scale is updated and views are notified.
		 */
		virtual void OnDisplayConfigChanged();

	public:
		typedef boost::signals2::signal<void(int, int)> displayConfigChangedSignal_t;
		displayConfigChangedSignal_t &GetDisplayConfigChangedSignal() { return displayConfigChangedSignal; }
	protected:
		void FireDisplayConfigChangedSignal(int width, int height) const;

	public:
		typedef boost::signals2::signal<void(double)> uiScaleChangedSignal_t;
		uiScaleChangedSignal_t &GetUiScaleChangedSignal() { return uiScaleChangedSignal; }
	protected:
		void FireUiScaleChangedSignal(double scale) const;

	public:
		/**
		 * Rendering for the frame has completed; update the screen.
		 */
		virtual void Flip() = 0;

		/**
		 * Take a screenshot.
		 */
		virtual void Screenshot() = 0;

	public:
		/**
		 * Retrieve the current UI origin coordinates.
		 * @return The coordinates, in UI-space.
		 */
		const Vec2 &GetUiOrigin() const { return uiOrigin; }

		/**
		 * Explicitly set the UI origin coordinates.
		 * All UI coordinates go through LayoutUiPosition will be translated by
		 * this offset.  In other words, this shifts where in UI-space the
		 * coordinates (0.0, 0.0) are.
		 * @param vec The coordinates (in UI-space).
		 */
		void SetUiOrigin(const Vec2 &vec) { uiOrigin = vec; }

		/**
		 * Shift the current UI origin coordinates by an offset.
		 * @param vec The offset (in UI-space).
		 * @return The old offset, so it can be restored later.
		 * @see #SetUiOrigin(const Vec2&)
		 */
		Vec2 AddUiOrigin(const Vec2 &vec)
		{
			Vec2 oldOrigin = uiOrigin;
			uiOrigin += vec;
			return oldOrigin;
		}

		/**
		 * Explicitly set the current UI layout flags.
		 * Calls to LayoutUiPosition combine the current layout flags with
		 * the widget layout flags when determining the adjusted position.
		 * @param flags The new flags.
		 */
		void SetUiLayoutFlags(uiLayoutFlags_t flags) { uiLayoutFlags = flags; }

		/**
		 * Combine the current UI layout flags with new flags.
		 * @param flags The new flags.
		 * @return The old flags.
		 * @see #SetUiLayoutFlags
		 */
		uiLayoutFlags_t AddUiLayoutFlags(uiLayoutFlags_t flags)
		{
			auto oldFlags = uiLayoutFlags;
			uiLayoutFlags |= flags;
			return oldFlags;
		}

		double GetUiScale() const { return uiScale; }
		const Vec2 &GetUiOffset() const { return uiOffset; }

		Vec2 LayoutUiPosition(const Vec2 &relPos, uiLayoutFlags_t layoutFlags=0)
		{
			Vec2 adjustedPos = relPos;
			adjustedPos += GetUiOrigin();
			adjustedPos *= GetUiScale();
			if (!((layoutFlags | uiLayoutFlags) & UiLayoutFlags::FLOATING)) {
				adjustedPos += GetUiOffset();
			}
			return adjustedPos;
		}

		Vec2 ScreenToUiPosition(const Vec2 &pos)
		{
			Vec2 adjustedPos = pos;
			adjustedPos -= GetUiOffset();
			adjustedPos /= GetUiScale();
			adjustedPos -= GetUiOrigin();
			return adjustedPos;
		}

		/**
		 * Retrieve the dimensions of the screen, in UI coordinates.
		 *
		 * This is useful for components with UiLayoutFlags::FLOATING set.
		 *
		 * @return The dimensions.
		 */
		const Vec2 &GetUiScreenSize() const { return uiScreenSize; }

	private:
		Vec2 uiOrigin;
		uiLayoutFlags_t uiLayoutFlags;
		double uiScale;
		Vec2 uiOffset;
		Vec2 uiScreenSize;
		displayConfigChangedSignal_t displayConfigChangedSignal;
		uiScaleChangedSignal_t uiScaleChangedSignal;

	public:
		struct styles_t {
			styles_t();

			// Standard text.
			UiFont bodyFont;
			Color bodyFg;
			UiFont bodyHeadFont;
			Color bodyHeadFg;
			UiFont bodyAsideFont;
			Color bodyAsideFg;

			// Announcements.
			UiFont announcementHeadFont;
			Color announcementHeadFg;
			UiFont announcementBodyFont;
			Color announcementBodyFg;
			Color announcementSymbolFg;
			Color announcementBg;

			// Console text.
			UiFont consoleFont;
			Color consoleFg;
			Color consoleCursorFg;
			Color consoleBg;

			// UI widgets (buttons, etc.).
			UiFont formFont;
			Color formFg;
			Color formDisabledFg;

			// Dialog background shading color.
			Color dialogBg;

			// Attributes for grid containers.
			Vec2 gridMargin;
			Vec2 gridPadding;

			// Colors specific to button-like widgets.
			Color buttonBg;
			Color buttonDisabledBg;
			Color buttonPressedBg;

			// Headings, titles.
			UiFont headingFont;
			Color headingFg;

			void Reload();
		} styles;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
