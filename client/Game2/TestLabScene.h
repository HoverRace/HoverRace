
// TestLabScene.h
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

#include "../../engine/Display/UiViewModel.h"

#include "GameDirector.h"

#include "FormScene.h"

namespace HoverRace {
	namespace Display {
		class Button;
		class ClickRegion;
		class Display;
		class FillBox;
		class Label;
		class ViewModel;
	}
}

namespace HoverRace {
namespace Client {

/**
 * A zoo of renderable components.
 * @author Michael Imamura
 */
class TestLabScene : public FormScene
{
	typedef FormScene SUPER;
	public:
		TestLabScene(Display::Display &display, GameDirector &director);
		virtual ~TestLabScene();

	private:
		template<class T>
		T *AddElem(T *elem)
		{
			elems.emplace_back(elem);
			elem->AttachView(display);
			return elem;
		}

		void AddAlignmentTestElem(Display::UiViewModel::Alignment::alignment_t alignment,
			const std::string &label, double x, double y);

	private:
		void OnDisplayConfigChanged();
		void OnDisplayInfoClicked();
		void OnMessageClicked();

	protected:
		virtual void OnPhaseChanged(Phase::phase_t oldPhase);

	public:
		virtual void Advance(Util::OS::timestamp_t tick);
		virtual void PrepareRender();
		virtual void Render();

	private:
		Display::Display &display;
		GameDirector &director;
		std::vector<std::unique_ptr<Display::ViewModel>> elems;

		boost::signals2::connection displayConfigChangedConn;
		boost::signals2::connection displayInfoClickedConn;
		boost::signals2::connection messageClickedConn;

		std::shared_ptr<Display::FillBox> displayInfoBox;
		std::shared_ptr<Display::Label> displayInfoLbl;
		std::shared_ptr<Display::ClickRegion> displayInfoBtn;
		/// Temporary box to highlight displayInfoBtn click area.
		std::shared_ptr<Display::FillBox> displayInfoBtnBox;
		std::shared_ptr<Display::Button> messageBtn;
};

}  // namespace Client
}  // namespace HoverRace
