
// VideoSettingsScene.h
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

#pragma once

#include "../../engine/Util/Config.h"

#include "SettingsScene.h"

namespace HoverRace {
	namespace Display {
		class Button;
		class Checkbox;
		class Label;
		class Slider;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Video settings dialog.
 * @author Michael Imamura
 */
class VideoSettingsScene : public SettingsScene
{
	using SUPER = SettingsScene;

public:
	VideoSettingsScene(Display::Display &display, GameDirector &director,
		const std::string &parentTitle);
	virtual ~VideoSettingsScene() { }

protected:
	void LoadFromConfig() override;
	void ResetToDefaults() override;

	void UpdateDisplayButton();

	void OnOk() override;
	void OnCancel() override;

	void OnDisplayClicked();

private:
	Util::Config::video_t &videoCfg;
	Util::Config::video_t origVideoCfg;

	std::shared_ptr<Display::Checkbox> fullscreenChk;
	std::shared_ptr<Display::Button> displayBtn;
	std::shared_ptr<Display::Slider> textScaleSlider;
	std::shared_ptr<Display::Label> textScalePreviewLbl;

	boost::signals2::scoped_connection fullscreenConn;
	boost::signals2::scoped_connection displayConn;
	boost::signals2::scoped_connection textScaleConn;
	boost::signals2::scoped_connection confirmOkConn;
	boost::signals2::scoped_connection displaySelConn;
};

}  // namespace Client
}  // namespace HoverRace
