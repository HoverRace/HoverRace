
// VideoSettingsScene.cpp
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

#include "../StdAfx.h"

#include "../../engine/Display/Label.h"
#include "../../engine/Display/Slider.h"
#include "../../engine/Util/Log.h"

#include "VideoSettingsScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

VideoSettingsScene::VideoSettingsScene(Display::Display &display,
	GameDirector &director) :
	SUPER(display, director, _("Video"), "Video Settings"),
	videoCfg(Config::GetInstance()->video), origVideoCfg(videoCfg)
{
	using namespace Display;
	const auto &s = display.styles;

	auto textScaleSlider = AddSetting(_("Text Scale"),
		new Slider(display, 0.5, 1.5, 0.1));
	textScaleSlider->SetSize(SLIDER_SIZE);
	textScaleSlider->SetValue(videoCfg.textScale);
	textScaleConn = textScaleSlider->GetValueChangedSignal().connect([&](double val) {
		this->textScalePreviewLbl->SetScale(val);
		videoCfg.textScale = val;
	});

	textScalePreviewLbl = AddSetting("",
		new Label(_("This is sample text."), s.bodyFont, s.bodyFg));
	textScalePreviewLbl->SetFixedScale(true);
	textScalePreviewLbl->SetScale(videoCfg.textScale);
}

void VideoSettingsScene::OnOk()
{
	Config::GetInstance()->Save();
	SUPER::OnOk();
}

void VideoSettingsScene::OnCancel()
{
	videoCfg = origVideoCfg;
	SUPER::OnCancel();
}

}  // namespace Client
}  // namespace HoverRace
