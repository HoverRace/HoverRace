
// BulletinBoard.cpp
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

#include "../StdAfx.h"

#include "../../engine/Display/FillBox.h"
#include "../../engine/Display/Label.h"

#include "Announcement.h"

#include "BulletinBoard.h"

namespace HoverRace {
namespace Client {

/// Displays a single announcement.
class BulletinBoard::Bulletin : public Display::Container /*{{{*/
{
	typedef Display::Container SUPER;
public:
	Bulletin(Display::Display &display, std::shared_ptr<Announcement> ann);
	virtual ~Bulletin() { }

private:
	std::shared_ptr<Announcement> ann;
	std::shared_ptr<Display::FillBox> bg;
	std::shared_ptr<Display::Label> labelLbl;
}; //}}}

BulletinBoard::BulletinBoard(Display::Display &display) :
	SUPER(display, Vec2(580, 720), false)
{
}

BulletinBoard::~BulletinBoard()
{
}

/**
 * Post an announcement to the bulletin board.
 * @param ann The announcement (may not be @c nullptr).
 */
void BulletinBoard::Announce(std::shared_ptr<Announcement> ann)
{
	bulletins.emplace_front(ann, AddChild(new Bulletin(display, ann)));
	//TODO: Update layout.
}

//{{{ Bulletin /////////////////////////////////////////////////////////////////

BulletinBoard::Bulletin::Bulletin(Display::Display &display,
	std::shared_ptr<Announcement> ann) :
	SUPER(display, Vec2(580, 100), false), ann(std::move(ann))
{
	using namespace Display;

	const auto &s = display.styles;

	bg = AddChild(new FillBox(580, 100, s.announcementBg));

	labelLbl = AddChild(new Label(420, this->ann->GetLabel(),
		s.announcementHeadFont, s.announcementHeadFg));
	labelLbl->SetPos(120, 20);

	auto icon = AddChild(this->ann->CreateIcon(display));
	icon->SetPos(40, 20);
	icon->SetSize(60, 60);

	//TODO: Player avatar.
}

//}}} Bulletin

}  // namespace HoverScript
}  // namespace Client
