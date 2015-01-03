
// TestLabScene.h
//
// Copyright (c) 2013-2015 Michael Imamura.
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
		class Display;
		class ScreenFade;
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
	using SUPER = FormScene;

public:
	TestLabScene(Display::Display &display, GameDirector &director,
		const std::string &startingModuleName = "");
	virtual ~TestLabScene();

public:
	class LabModule;
	class ModuleButtonBase;
	void AddModuleButton(ModuleButtonBase *btn);

public:
	void OnScenePushed() override;
	void PrepareRender() override;
	void Render() override;

private:
	const std::string startingModuleName;
	double btnPosY;
	std::unique_ptr<Display::ScreenFade> fader;
	std::shared_ptr<ModuleButtonBase> startingModuleBtn;
};

}  // namespace Client
}  // namespace HoverRace
