
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
namespace Client {

/**
 * Video settings dialog.
 * @author Michael Imamura
 */
class VideoSettingsScene : public SettingsScene
{
	using SUPER = SettingsScene;

public:
	VideoSettingsScene(Display::Display &display, GameDirector &director);
	virtual ~VideoSettingsScene() { }

protected:
	void OnOk() override;
	void OnCancel() override;

private:
	Util::Config::cfg_video_t &videoCfg;
	const Util::Config::cfg_video_t origVideoCfg;

	boost::signals2::scoped_connection textScaleConn;
};

}  // namespace Client
}  // namespace HoverRace
