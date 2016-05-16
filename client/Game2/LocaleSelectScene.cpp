
// LocaleSelectScene.cpp
//
// Copyright (c) 2015-2016 Michael Imamura.
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

#include <boost/algorithm/string/predicate.hpp>

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
	SUPER(display, director, JoinTitles(parentTitle, _("SELECT LANGUAGE")),
		"Locale Select")
{
	using namespace Display;
	using Alignment = UiViewModel::Alignment;

	SupportCancelAction();

	auto root = GetContentRoot();

	localeList = root->NewChild<PickList<std::string>>(display,
		BasePickList::Mode::LIST, Vec2(260, 520));
	localeList->SetPos(640, 0);
	localeList->SetAlignment(Alignment::N);

	localeList->Add(_("Auto-detect"), "");

	// Produce a list sorted by user-friendly name, instead of by locale ID.
	using item_t = const Util::Locale::value_type*;
	std::vector<item_t> sorted;
	for (const auto &loc : locale) {
		sorted.push_back(&loc);
	}
	std::sort(sorted.begin(), sorted.end(),
		[](const item_t &a, const item_t &b) {
			return boost::algorithm::ilexicographical_compare(
				a->second, b->second);
		});
	for (const auto &loc : sorted) {
		localeList->Add(loc->second, loc->first);
	}

	localeList->GetValueChangedSignal().connect(
		std::bind(&LocaleSelectScene::OnLocaleSelected, this));
	localeList->RequestFocus();
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
