
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

#include "UiScene.h"

namespace HoverRace {
	namespace Display {
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
class TestLabScene : public UiScene
{
	typedef UiScene SUPER;
	public:
		TestLabScene(Display::Display &display);
		virtual ~TestLabScene();

	private:
		template<class T>
		T *AddElem(T *elem)
		{
			elems.emplace_back(elem);
			elem->AttachView(display);
			return elem;
		}

	private:
		void OnDisplayConfigChanged();

	protected:
		virtual void OnPhaseChanged(Phase::phase_t oldPhase);

	public:
		virtual void Advance(Util::OS::timestamp_t tick);
		virtual void PrepareRender();
		virtual void Render();

	private:
		Display::Display &display;
		std::vector<std::unique_ptr<Display::ViewModel>> elems;
		boost::signals2::connection displayConfigChangedConn;
		Display::FillBox *displayInfoBox;
		Display::Label *displayInfoLbl;
};

}  // namespace Client
}  // namespace HoverRace
