
// MessageAnnouncement.cpp
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

#include "../../engine/Display/Display.h"
#include "../../engine/Display/Container.h"
#include "../../engine/Display/FlexGrid.h"
#include "../../engine/Display/Label.h"
#include "../../engine/Display/SymbolIcon.h"

#include "MessageAnnouncement.h"

namespace HoverRace {
namespace Client {

MessageAnnouncement::MessageAnnouncement(const std::string &label,
	const std::string &text, std::shared_ptr<Player::Player> player) :
	SUPER("message", label, player),
	text(text)
{
}

std::shared_ptr<Display::FillBox> MessageAnnouncement::CreateIcon(
	Display::Display &display, Display::Container &parent) const
{
	const auto &s = display.styles;

	// Info icon (will be resized by the container).
	return parent.NewChild<Display::SymbolIcon>(
		1, 1, 0xf05a, s.announcementSymbolFg);
}

void MessageAnnouncement::CreateContents(Display::Display &display,
	Display::FlexGrid &grid) const
{
	if (text.empty()) return;

	const auto &s = display.styles;

	grid.At(0, 0).NewChild<Display::Label>(
		text, s.announcementBodyFont, s.announcementBodyFg);
}

}  // namespace Client
}  // namespace HoverRace
