
// AudioSettingsScene.h
//
// Copyright (c) 2014, 2016 Michael Imamura.
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

#include "../../engine/Util/Config.h"

#include "SettingsScene.h"

namespace HoverRace {
	namespace Display {
		class Slider;
	}
	namespace VideoServices {
		class ShortSound;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Audio settings dialog.
 * @author Michael Imamura
 */
class AudioSettingsScene : public SettingsScene
{
	using SUPER = SettingsScene;

public:
	AudioSettingsScene(Display::Display &display, GameDirector &director,
		const std::string &parentTitle);
	virtual ~AudioSettingsScene() { }

protected:
	void LoadFromConfig() override;
	void ResetToDefaults() override;

private:
	static VideoServices::ShortSound *LoadSound(int id);

protected:
	void OnOk() override;
	void OnCancel() override;

private:
	Util::Config::audio_t &audioCfg;
	Util::Config::audio_t origAudioCfg;

	bool skipSoundTest;
	VideoServices::ShortSound *testSound;

	std::shared_ptr<Display::Slider> sfxVolumeSlider;

	boost::signals2::scoped_connection sfxVolumeConn;
};

}  // namespace Client
}  // namespace HoverRace
