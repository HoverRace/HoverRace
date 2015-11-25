
// LocaleSelectScene.cpp
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
#include "../../engine/Display/Container.h"
#include "../../engine/Util/Locale.h"

#include "LocaleSelectScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

LocaleSelectScene::LocaleSelectScene(Display::Display &display,
	GameDirector &director,
	const std::string &parentTitle, const Util::Locale &locale,
	const std::string&) :
	SUPER(display, director, JoinTitles(parentTitle, _("Select Language")),
		"Locale Select")
{
	using namespace Display;
	using Alignment = UiViewModel::Alignment;

	SupportCancelAction();

	auto root = GetContentRoot();

	localeList = root->NewChild<PickList<std::string>>(display, Vec2(260, 520));
	localeList->SetPos(640, 0);
	localeList->SetAlignment(Alignment::N);

	localeList->Add(_("Auto-detect"), "");
	for (const auto &loc : locale) {
		localeList->Add(loc.second, loc.first);
	}

	localeList->GetValueChangedSignal().connect(
		std::bind(&LocaleSelectScene::OnLocaleSelected, this));
}

std::string LocaleSelectScene::GetLocaleId() const
{
	if (auto id = localeList->GetValue()) {
		return *id;
	}
	else {
		return "";
	}
}

void LocaleSelectScene::OnLocaleSelected()
{
	confirmSignal();
	director.RequestPopScene();
}

}  // namespace Client
}  // namespace HoverRace
