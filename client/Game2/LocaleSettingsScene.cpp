
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
#include "MessageScene.h"

#include "LocaleSettingsScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

LocaleSettingsScene::LocaleSettingsScene(Display::Display &display,
	GameDirector &director, const std::string &parentTitle) :
	SUPER(display, director, parentTitle, _("Language and Units"),
		"Locale Settings"),
	locale(Config::GetInstance()->GetLocale()),
	i18nCfg(Config::GetInstance()->i18n), origI18nCfg(i18nCfg)
{
	using namespace Display;

	const auto &videoCfg = Config::GetInstance()->video;

	langBtn = AddSetting(_("Language")).
		NewChild<Button>(display, " ")->GetContents();
	langBtn->SetFixedWidth(std::min(960.0, 640.0 * videoCfg.textScale));
	langConn = langBtn->GetClickedSignal().connect(std::bind(
		&LocaleSettingsScene::OnLangClicked, this));

	GetSettingsGrid()->RequestFocus();
}

void LocaleSettingsScene::LoadFromConfig()
{
	auto preferredLocale = i18nCfg.preferredLocale;
	if (preferredLocale.empty()) {
		std::ostringstream oss;
		oss << _("Auto-detect");
		if (auto selLocale = locale.GetSelectedLocaleId()) {
			oss << " [" << locale.IdToName(*selLocale) << "]";
		}
		preferredLocale = oss.str();
	}
	else {
		preferredLocale = locale.IdToName(preferredLocale);
	}

	langBtn->SetText(preferredLocale);
}

void LocaleSettingsScene::ResetToDefaults()
{
	i18nCfg.ResetToDefaults();
}

void LocaleSettingsScene::OnOk()
{
	if (origI18nCfg.preferredLocale != i18nCfg.preferredLocale) {
		auto confirmScene = NewSoftRestartConfirmScene();
		confirmOkConn = confirmScene->GetOkSignal().connect([&]() {
			locale.RequestPreferredLocale();

			Config::GetInstance()->Save();
			director.RequestSoftRestart();
		});
		director.RequestPushScene(confirmScene);
	}
	else {
		Config::GetInstance()->Save();
		SUPER::OnOk();
	}
}

void LocaleSettingsScene::OnCancel()
{
	i18nCfg = origI18nCfg;
	SUPER::OnCancel();
}

void LocaleSettingsScene::OnLangClicked()
{
	auto scene = std::make_shared<LocaleSelectScene>(display, director,
		GetTitle(), locale, i18nCfg.preferredLocale);

	auto sp = scene.get();  // Prevent circular reference.
	langSelConn = scene->GetConfirmSignal().connect([=]() {
		i18nCfg.preferredLocale = sp->GetLocaleId();

		RequestLoadFromConfig();
	});

	director.RequestPushScene(scene);
}

}  // namespace Client
}  // namespace HoverRace
