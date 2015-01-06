
// SettingsScene.h
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

#pragma once

#include "../../engine/Display/FlexGrid.h"

#include "DialogScene.h"

namespace HoverRace {
namespace Client {

/**
 * Base for settings pages.
 * @author Michael Imamura
 */
class SettingsScene : public DialogScene
{
	using SUPER = DialogScene;

public:
	SettingsScene(Display::Display &display, GameDirector &director,
		const std::string &parentTitle, const std::string &title,
		const std::string &name);
	virtual ~SettingsScene() { }

protected:
	/// Refresh the form widgets with values from the config.
	virtual void LoadFromConfig() = 0;

	/// Indicate that the form needs to be refreshed from the config.
	void RequestLoadFromConfig()
	{
		needsLoadFromConfig = true;
	}

	/// Reset the config values to their defaults (if possible).
	virtual void ResetToDefaults() = 0;

public:
	Display::FlexGrid::CellProxy AddSetting(const std::string &label);

protected:
	void OnExtra() override;

public:
	void PrepareRender() override;

protected:
	static Vec2 SLIDER_SIZE;

private:
	std::shared_ptr<Display::FlexGrid> settingsGrid;
	size_t curRow;
	bool needsLoadFromConfig;
	boost::signals2::scoped_connection confirmOkConn;
};

}  // namespace Client
}  // namespace HoverRace
