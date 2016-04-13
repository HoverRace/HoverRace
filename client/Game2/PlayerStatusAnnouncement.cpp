
// PlayerStatusAnnouncement.cpp
//
// Copyright (c) 2016 Michael Imamura.
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

#include "../../engine/Display/Container.h"
#include "../../engine/Display/SymbolIcon.h"
#include "../../engine/Player/Player.h"
#include "../../engine/Util/Symbol.h"

#include "PlayerStatusAnnouncement.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

/**
 * Constructor.
 * @param status The player status.
 * @param target The target player (may not be @c nullptr).
 * @param subject The subject player (may not be @c nullptr).
 */
PlayerStatusAnnouncement::PlayerStatusAnnouncement(Status::status_t status,
	std::shared_ptr<Player::Player> target,
	std::shared_ptr<Player::Player> subject) :
	SUPER(subject->GetName(), RenderStatusText(status, *subject), target),
	subject(subject)
{
}

/**
 * Generate the announcement text for the status.
 * @param status The player status.
 * @param subject The subject player.
 */
std::string PlayerStatusAnnouncement::RenderStatusText(Status::status_t status,
	const Player::Player &subject)
{
	auto profileName = subject.GetProfile()->GetName();

	switch (status) {
		case Status::PRESENT:
			return boost::str(boost::format(_("Welcome, %s!")) %
				profileName);

		case Status::JOINED:
			return boost::str(boost::format(_("%s has joined")) %
				profileName);

		default:
			throw UnimplementedExn(boost::str(
				boost::format("PlayerStatusAnnouncement::GetStatusText(%d)") %
				status));
	}
}

std::shared_ptr<Display::FillBox> PlayerStatusAnnouncement::CreateIcon(
	Display::Display &display, Display::Container &parent) const
{
	const auto &s = display.styles;

	//TODO: Use Picture when Picture extends FillBox.

	// Info icon (will be resized by the container).
	return parent.NewChild<Display::SymbolIcon>(
		1, 1, Symbol::INFO_CIRCLE, s.announcementSymbolFg);
}

}  // namespace Client
}  // namespace HoverRace
