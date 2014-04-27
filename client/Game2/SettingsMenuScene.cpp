
// SettingsMenuScene.cpp
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

#include "StdAfx.h"

#include "../../engine/Control/Controller.h"
#include "../../engine/Display/Button.h"
#include "../../engine/Display/Display.h"
#include "../../engine/Display/FlexGrid.h"
#include "../../engine/Display/ScreenFade.h"
#include "../../engine/Util/Log.h"

#include "AudioSettingsScene.h"
#include "GameDirector.h"

#include "SettingsMenuScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

namespace {
	template<class T>
	class MenuItemButton : public Display::Button
	{
		typedef Display::Button SUPER;
		public:
			MenuItemButton(Display::Display &display, GameDirector &director,
				const std::string &label, bool enabled=true) :
				SUPER(display, label)
			{
				SetEnabled(enabled);

				clickedConn = GetClickedSignal().connect([&](Display::ClickRegion&) {
					director.RequestPushScene(std::make_shared<T>(display, director));
				});
			}
			virtual ~MenuItemButton() { }

		private:
			boost::signals2::scoped_connection clickedConn;
	};
}

SettingsMenuScene::SettingsMenuScene(Display::Display &display,
                                     GameDirector &director) :
	SUPER(display, "Settings", "Settings")
{
	typedef Display::UiViewModel::Alignment Alignment;

	SetPhaseTransitionDuration(200);

	fader.reset(new Display::ScreenFade(0xcc000000, 0.0));
	fader->AttachView(display);

	auto root = GetContentRoot();

	menuGrid = root->AddChild(new Display::FlexGrid(display));
	menuGrid->SetPos(640, 140);
	menuGrid->SetAlignment(Alignment::N);

	menuGrid->GetColumnDefault(0).SetFill(true);

	size_t row = 0;
	menuGrid->AddGridCell(row++, 0,
		new MenuItemButton<AudioSettingsScene>(display, director, "Profile", false));
	menuGrid->AddGridCell(row++, 0,
		new MenuItemButton<AudioSettingsScene>(display, director, "Audio"));
	menuGrid->AddGridCell(row++, 0,
		new MenuItemButton<AudioSettingsScene>(display, director, "Video", false));
	menuGrid->AddGridCell(row++, 0,
		new MenuItemButton<AudioSettingsScene>(display, director, "Network", false));
	menuGrid->AddGridCell(row++, 0,
		new MenuItemButton<AudioSettingsScene>(display, director, "Advanced", false));
}

SettingsMenuScene::~SettingsMenuScene()
{
}

void SettingsMenuScene::OnPhaseTransition(double progress)
{
	fader->SetOpacity(progress);
}

void SettingsMenuScene::PrepareRender()
{
	fader->PrepareRender();

	SUPER::PrepareRender();
}

void SettingsMenuScene::Render()
{
	fader->Render();

	if (GetPhase() != Phase::STOPPING) {
		SUPER::Render();
	}
}

}  // namespace Client
}  // namespace HoverRace
