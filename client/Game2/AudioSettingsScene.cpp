
// AudioSettingsScene.cpp
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

#include "AudioSettingsScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

AudioSettingsScene::AudioSettingsScene(Display::Display &display,
	GameDirector &director) :
	SUPER(display, director, _("Audio"), "Audio Settings"),
	audioCfg(Config::GetInstance()->audio), origAudioCfg(audioCfg)
{
	using namespace Display;

	auto sfxVolumeSlider = AddSetting(_("Sound Effects"),
		new Slider(display, 0, 1.0, 0.1));
	sfxVolumeSlider->SetSize(SLIDER_SIZE);
	sfxVolumeSlider->SetValue(audioCfg.sfxVolume);
	sfxVolumeSlider->GetValueChangedSignal().connect([&](double val) {
		audioCfg.sfxVolume = val;
	});
}

void AudioSettingsScene::OnOk()
{
	Config::GetInstance()->Save();
	SUPER::OnOk();
}

void AudioSettingsScene::OnCancel()
{
	audioCfg = origAudioCfg;
	SUPER::OnCancel();
}

}  // namespace Client
}  // namespace HoverRace
