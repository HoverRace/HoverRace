
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

#include "../../engine/Display/Slider.h"
#include "../../engine/ObjFac1/ObjFac1Res.h"
#include "../../engine/ObjFacTools/ResourceLib.h"
#include "../../engine/Util/DllObjectFactory.h"
#include "../../engine/Util/Log.h"
#include "../../engine/VideoServices/SoundServer.h"

#include "AudioSettingsScene.h"

using namespace HoverRace::Util;
namespace SoundServer = HoverRace::VideoServices::SoundServer;

namespace HoverRace {
namespace Client {

AudioSettingsScene::AudioSettingsScene(Display::Display &display,
	GameDirector &director) :
	SUPER(display, director, _("Audio"), "Audio Settings"),
	audioCfg(Config::GetInstance()->audio), origAudioCfg(audioCfg),
	testSound(LoadSound(MR_SND_START))
{
	using namespace Display;

	sfxVolumeSlider = AddSetting(_("Sound Effects"),
		new Slider(display, 0, 1.0, 0.1));
	sfxVolumeSlider->SetSize(SLIDER_SIZE);
	sfxVolumeConn = sfxVolumeSlider->GetValueChangedSignal().connect([&](double val) {
		audioCfg.sfxVolume = val;
		SoundServer::Play(testSound);
	});
}

void AudioSettingsScene::LoadFromConfig()
{
	origAudioCfg = audioCfg;
	sfxVolumeSlider->SetValue(audioCfg.sfxVolume);
}

void AudioSettingsScene::ResetToDefaults()
{
	audioCfg.ResetToDefaults();
}

/**
 * Loads a sound from the resources.
 * @param id The resource ID.
 * @return The sound effect, or @c nullptr if it fails to load.
 */
VideoServices::ShortSound *AudioSettingsScene::LoadSound(int id)
{
	auto res = DllObjectFactory::GetResourceLib().GetShortSound(id);
	if (!res) {
		HR_LOG(warning) << "Unable to load sound ID: " << id;
		return nullptr;
	}

	return res->GetSound();
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
