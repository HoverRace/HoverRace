
// Announcement.h
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

namespace HoverRace {
	namespace Display {
		class Display;
		class FillBox;
		class FlexGrid;
	}
	namespace Player {
		class Player;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Base class for announcements.
 * @author Michael Imamura
 */
class Announcement
{
public:
	/**
	 * Constructor.
	 * @param postType The type of announcement, used for debugging.
	 * @param label The user-visible label.
	 * @param player The optional target player this announcement is
	 *               aimed at.
	 */
	Announcement(const std::string &postType, const std::string &label,
		std::shared_ptr<Player::Player> player) :
		postType(postType), label(label), player(std::move(player)) { }
	virtual ~Announcement() { }

	Announcement &operator=(const Announcement&) = delete;

public:
	virtual std::ostream &StreamOut(std::ostream &os) const
	{
		os << label;
		return os;
	}

public:
	const std::string &GetPostType() const { return postType; }
	const std::string &GetLabel() const { return label; }
	std::shared_ptr<Player::Player> GetPlayer() const { return player; }

public:
	/**
	 * Create the optional icon to display next to the announcement.
	 * @param display The display the announcement will appear on.
	 * @param container The container the icon will be added to.
	 * @return The created icon (may be @c nullptr).
	 */
	virtual std::shared_ptr<Display::FillBox> CreateIcon(
		Display::Display &display, Display::Container &parent) const = 0;

	/**
	 * Fill in the UI contents of the announcement.
	 * @param display The display the announcement will appear on.
	 * @param grid The grid (initially empty) to which components should
	 *             be added.
	 */
	virtual void CreateContents(Display::Display &display,
		Display::FlexGrid *grid) const = 0;

	/**
	 * Handle when the announcement is clicked.
	 */
	virtual void OnClick() { }

private:
	const std::string postType;
	const std::string label;
	std::shared_ptr<Player::Player> player;
};

inline std::ostream &operator<<(std::ostream &os, const Announcement &ann)
{
	return ann.StreamOut(os);
}

}  // namespace Client
}  // namespace HoverRace
