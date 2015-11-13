
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

#include "../../engine/Display/Button.h"
#include "../../engine/Util/Locale.h"

#include "LocaleSelectScene.h"

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

	langBtn = AddSetting(_("Language")).
		NewChild<Button>(display, " ")->GetContents();
	langConn = langBtn->GetClickedSignal().connect(std::bind(
		&LocaleSettingsScene::OnLangClicked, this));
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

	langBtn->SetText(preferredLocale);
}

void LocaleSettingsScene::ResetToDefaults()
{
	localeCfg.ResetToDefaults();
}

void LocaleSettingsScene::OnLangClicked()
{
	auto scene = std::make_shared<LocaleSelectScene>(display, director,
		GetTitle(), localeCfg, localeCfg.GetPreferredLocale());

	/*TODO
	auto sp = scene.get();  // Prevent circular reference.
	langSelConn = scene->GetOkSignal().connect([=]() {
		localeCfg.SetPreferredLocale(sp->GetLocaleId());
		RequestLoadFromConfig();
	});
	*/

	director.RequestPushScene(scene);
}

}  // namespace Client
}  // namespace HoverRace
