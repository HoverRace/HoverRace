
// Display.h
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
#include "ViewAttacher.h"

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
		class Label;
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
	public ViewAttacher<Label>
{
	public:
		Display() : uiOrigin(0, 0) { }
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
		 * @note Subclasses should call FireDisplayConfigChangedSignal(int, int)
		 *       to notify views.
		 */
		virtual void OnDisplayConfigChanged() = 0;

	public:
		typedef boost::signals2::signal<void(int, int)> displayConfigChangedSignal_t;
		displayConfigChangedSignal_t &GetDisplayConfigChangedSignal() { return displayConfigChangedSignal; }
	protected:
		void FireDisplayConfigChangedSignal(int width, int height) const;

	public:
		/**
		 * Rendering for the frame has completed; update the screen.
		 */
		virtual void Flip() = 0;

	public:
		Vec2 GetUiOrigin() const
		{
			return uiOrigin;
		}

		void SetUiOrigin(const Vec2 &vec)
		{
			uiOrigin = vec;
		}

		Vec2 AddUiOrigin(const Vec2 &vec)
		{
			Vec2 oldOrigin = uiOrigin;
			uiOrigin += vec;
			return oldOrigin;
		}

	private:
		Vec2 uiOrigin;
		displayConfigChangedSignal_t displayConfigChangedSignal;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
