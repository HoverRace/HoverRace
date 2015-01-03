
// VideoSettingsScene.cpp
//
// Copyright (c) 2014, 2015 Michael Imamura.
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

#include "../../engine/Display/Button.h"
#include "../../engine/Display/Checkbox.h"
#include "../../engine/Display/Label.h"
#include "../../engine/Display/Slider.h"
#include "../../engine/Util/Log.h"
#include "../../engine/Util/OS.h"

#include "DisplaySelectScene.h"
#include "MessageScene.h"
#include "VideoSettingsScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

VideoSettingsScene::VideoSettingsScene(Display::Display &display,
	GameDirector &director, const std::string &parentTitle) :
	SUPER(display, director, parentTitle, _("Video"), "Video Settings"),
	videoCfg(Config::GetInstance()->video), origVideoCfg(videoCfg),
	splitModeGroup()
{
	using namespace Display;
	const auto &s = display.styles;

	fullscreenChk = AddSetting(_("Display Mode"),
		new Checkbox(display, _("Fullscreen")));
	fullscreenConn = fullscreenChk->GetClickedSignal().connect([&](ClickRegion&) {
		videoCfg.fullscreen = !videoCfg.fullscreen;
	});

	displayBtn = AddSetting(_("Fullscreen Resolution"),
		new Button(display, "Select"));
	displayConn = displayBtn->GetClickedSignal().connect(std::bind(
		&VideoSettingsScene::OnDisplayClicked, this));

	textScaleSlider = AddSetting(_("Text Scale"),
		new Slider(display, 0.5, 1.5, 0.1));
	textScaleSlider->SetSize(SLIDER_SIZE);
	textScaleConn = textScaleSlider->GetValueChangedSignal().connect([&](double val) {
		this->textScalePreviewLbl->SetScale(val / 2.0);
		videoCfg.textScale = val;
	});

	// Use a 2x scaled font so the >1.x preview doesn't get blurred / pixelated.
	auto scaledFont = s.bodyFont;
	scaledFont.size *= 2;

	textScalePreviewLbl = AddSetting("",
		new Label(_("This is sample text."), scaledFont, s.bodyFg));
	textScalePreviewLbl->SetFixedScale(true);
	textScalePreviewLbl->SetScale(videoCfg.textScale / 2.0);

	auto splitModeGrid = AddSetting(_("Split-screen mode"),
		new FlexGrid(display));
	splitModeGroup.Add(
		splitModeGrid->AddGridCell(0, 0,
			new RadioButton<bool>(display, _("Stacked"), true))->GetContents());
	splitModeGroup.Add(
		splitModeGrid->AddGridCell(0, 1,
			new RadioButton<bool>(display, _("Side-by-side"), false))->
				GetContents());
}

void VideoSettingsScene::LoadFromConfig()
{
	fullscreenChk->SetChecked(videoCfg.fullscreen);
	UpdateDisplayButton();
	textScaleSlider->SetValue(videoCfg.textScale);
	splitModeGroup.SetValue(videoCfg.stackedSplitscreen);
}

void VideoSettingsScene::ResetToDefaults()
{
	videoCfg.ResetToDefaults();
}

void VideoSettingsScene::UpdateDisplayButton()
{
	auto dispStr = boost::str(
		boost::format(_("Monitor %d: %dx%d"), OS::stdLocale) %
			(videoCfg.fullscreenMonitorIndex + 1) %
			videoCfg.xResFullscreen % videoCfg.yResFullscreen);
	displayBtn->SetText(dispStr);
}

void VideoSettingsScene::OnOk()
{
	const bool needsSoftRestart =
		origVideoCfg.fullscreen != videoCfg.fullscreen ||
		origVideoCfg.fullscreenMonitorIndex != videoCfg.fullscreenMonitorIndex ||
		origVideoCfg.xResFullscreen != videoCfg.xResFullscreen ||
		origVideoCfg.yResFullscreen != videoCfg.yResFullscreen ||
		origVideoCfg.fullscreenRefreshRate != videoCfg.fullscreenRefreshRate;
	const bool needsMainMenu =
		origVideoCfg.textScale != videoCfg.textScale ||
		origVideoCfg.stackedSplitscreen != videoCfg.stackedSplitscreen;

	if (needsSoftRestart) {
		auto confirmScene = std::make_shared<MessageScene>(display, director,
			_("Settings changed"),
			std::string(_("To apply these changes, the game must restart.")) +
			"\n\n" +
			_("This will abandon any race in progress."),
			true);
		confirmOkConn = confirmScene->GetOkSignal().connect([&]() {
			Config::GetInstance()->Save();
			director.RequestSoftRestart();
		});
		director.RequestPushScene(confirmScene);
	}
	else if (needsMainMenu) {
		auto confirmScene = std::make_shared<MessageScene>(display, director,
			_("Settings changed"),
			std::string(_("To apply these changes, the game must return "
			"to the main menu.")) +
			"\n\n" +
			_("This will abandon any race in progress."),
			true);
		confirmOkConn = confirmScene->GetOkSignal().connect([&]() {
			Config::GetInstance()->Save();
			director.RequestMainMenu();
		});
		director.RequestPushScene(confirmScene);
	}
	else {
		Config::GetInstance()->Save();
		SUPER::OnOk();
	}
}

void VideoSettingsScene::OnCancel()
{
	videoCfg = origVideoCfg;
	SUPER::OnCancel();
}

void VideoSettingsScene::OnDisplayClicked()
{
	auto scene = std::make_shared<DisplaySelectScene>(display, director,
		GetTitle(),
		videoCfg.fullscreenMonitorIndex,
		videoCfg.xResFullscreen,
		videoCfg.yResFullscreen,
		videoCfg.fullscreenRefreshRate);

	displaySelConn = scene->GetOkSignal().connect([=]() {
		videoCfg.fullscreenMonitorIndex = scene->GetMonitorIdx();

		const auto &res = scene->GetResolution();
		videoCfg.xResFullscreen = res.xRes;
		videoCfg.yResFullscreen = res.yRes;
		videoCfg.fullscreenRefreshRate = res.refreshRate;

		RequestLoadFromConfig();
	});

	director.RequestPushScene(scene);
}

}  // namespace Client
}  // namespace HoverRace
