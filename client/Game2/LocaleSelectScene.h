
// LocaleSelectScene.h
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

#pragma once

#include "../../engine/Display/PickList.h"

#include "DialogScene.h"

namespace HoverRace {
	namespace Client {
		class GameDirector;
	}
	namespace Display {
		class Display;
	}
	namespace Util {
		class Locale;
	}
}

namespace HoverRace {
namespace Client {

/**
* Selector for locale.
* @author Michael Imamura
*/
class LocaleSelectScene : public DialogScene
{
	using SUPER = DialogScene;

public:
	LocaleSelectScene(Display::Display &display, GameDirector &director,
		const std::string &parentTitle, const Util::Locale &locale,
		const std::string &localeId);
	virtual ~LocaleSelectScene() { }

public:
	std::string GetLocaleId() const;

public:
	void OnLocaleSelected();

public:
	using confirmSignal_t = boost::signals2::signal<void()>;
	confirmSignal_t &GetConfirmSignal() { return confirmSignal; }

private:
	std::shared_ptr<Display::PickList<std::string>> localeList;

	confirmSignal_t confirmSignal;
};

}  // namespace Client
}  // namespace HoverRace
