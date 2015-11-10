
// LocaleSettingsScene.cpp
//
// Copyright (c) 2015 Michael Imamura.
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

#include "../../engine/Display/Label.h"
#include "../../engine/Util/Locale.h"

#include "LocaleSettingsScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

LocaleSettingsScene::LocaleSettingsScene(Display::Display &display,
	GameDirector &director, const std::string &parentTitle) :
	SUPER(display, director, parentTitle, _("Language and Units"),
		"Locale Settings"),
	localeCfg(Config::GetInstance()->GetLocale())
{
	using namespace Display;
	const auto &s = display.styles;

	localeLbl = AddSetting(_("Language")).
		NewChild<Label>(" ", s.bodyFont, s.bodyFg)->GetContents();
}

void LocaleSettingsScene::LoadFromConfig()
{
	auto preferredLocale = localeCfg.GetPreferredLocale();
	if (preferredLocale.empty()) {
		std::ostringstream oss;
		oss << _("Auto-detect");
		if (auto selLocale = localeCfg.GetSelectedLocaleId()) {
			oss << " [" << localeCfg.IdToName(*selLocale) << "]";
		}
		preferredLocale = oss.str();
	}

	localeLbl->SetText(preferredLocale);
}

void LocaleSettingsScene::ResetToDefaults()
{
	localeCfg.ResetToDefaults();
}

}  // namespace Client
}  // namespace HoverRace
