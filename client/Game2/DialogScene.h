
// DialogScene.h
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

#include "GameDirector.h"

#include "FormScene.h"

namespace HoverRace {
	namespace Display {
		class ActionButton;
		class FlexGrid;
		class Background;
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
	using SUPER = FormScene;

public:
	DialogScene(Display::Display &display, GameDirector &director,
		const std::string &title = "", const std::string &name = "");
	virtual ~DialogScene();

protected:
	const std::string &GetTitle() const { return title; }

	void SetStoppingTransitionEnabled(bool enabled);

	Display::Container *GetContentRoot() const { return contentRoot.get(); }
	Display::Container *GetStatusRoot() const { return statusRoot.get(); }

	void SupportOkAction(const std::string &label = "");
	void SupportCancelAction(const std::string &label = "");
	void SupportExtraAction(const std::string &label);

	void SetBackground(Display::Background *fader);

protected:
	virtual void OnOk();
	virtual void OnCancel();
	virtual void OnExtra();

protected:
	static std::string JoinTitles(const std::string &parent,
		const std::string &title);

public:
	void AttachController(Control::InputEventController &controller,
		ConnList &conns) override;
	void DetachController(Control::InputEventController &controller,
		ConnList &conns) override;
	void OnPhaseTransition(double progress) override;
	void PrepareRender() override;
	void Render() override;

public:
	static constexpr double MARGIN_WIDTH = 40;

protected:
	GameDirector &director;

private:
	std::string title;
	bool stoppingTransitionEnabled;
	size_t actionGridCol;

	std::unique_ptr<Display::Background> fader;
	std::shared_ptr<Display::Container> contentRoot;
	std::shared_ptr<Display::Container> statusRoot;
	std::shared_ptr<Display::FlexGrid> actionGrid;
	std::shared_ptr<Display::ActionButton> okBtn;
	std::shared_ptr<Display::ActionButton> cancelBtn;
	std::shared_ptr<Display::ActionButton> extraBtn;
};

}  // namespace Client
}  // namespace HoverRace
