
// DialogScene.h
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

#include "FormScene.h"

namespace HoverRace {
	namespace Display {
		class ScreenFade;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Base class for scenes with a title and status area.
 * @author Michael Imamura
 */
class DialogScene : public FormScene
{
	typedef FormScene SUPER;
	public:
		DialogScene(Display::Display &display, const std::string &title="",
			const std::string &name="");
		virtual ~DialogScene();

	protected:
		Display::Container *GetContentRoot() const { return contentRoot.get(); }
		Display::Container *GetStatusRoot() const { return statusRoot.get(); }

	public:
		virtual void OnPhaseTransition(double progress) override;
		virtual void PrepareRender() override;
		virtual void Render() override;

	public:
		static const double MARGIN_WIDTH;
	private:
		std::string title;
		std::unique_ptr<Display::ScreenFade> fader;
		std::shared_ptr<Display::Container> contentRoot;
		std::shared_ptr<Display::Container> statusRoot;
};

}  // namespace Client
}  // namespace HoverRace
