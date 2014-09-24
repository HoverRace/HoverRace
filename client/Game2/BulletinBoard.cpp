
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

#include "../../engine/Display/ClickRegion.h"
#include "../../engine/Display/FillBox.h"
#include "../../engine/Display/FlexGrid.h"
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
	Bulletin(BulletinBoard *board, Display::Display &display,
		std::shared_ptr<Announcement> ann);
	virtual ~Bulletin() { }

public:
	Announcement *GetAnnouncement() const { return ann.get(); }

protected:
	void Layout() override;

private:
	BulletinBoard *board;
	std::shared_ptr<Announcement> ann;
	std::shared_ptr<Display::FillBox> bg;
	std::shared_ptr<Display::ClickRegion> clickBox;
	std::shared_ptr<Display::Label> labelLbl;
	std::shared_ptr<Display::FlexGrid> contentGrid;
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
	bulletins.emplace_front(AddChild(new Bulletin(this, display, ann)));
	RequestLayout();
}

void BulletinBoard::Layout()
{
	Vec2 pos{ 0, 0 };
	for (auto &bulletin : bulletins) {
		if (bulletin->IsVisible()) {
			bulletin->SetPos(pos);
			pos.y += bulletin->GetSize().y;
		}
	}
}

//{{{ Bulletin /////////////////////////////////////////////////////////////////

BulletinBoard::Bulletin::Bulletin(BulletinBoard *board,
	Display::Display &display, std::shared_ptr<Announcement> announcement) :
	SUPER(display, Vec2(580, 0), false),
	board(board), ann(std::move(announcement))
{
	using namespace Display;

	const auto &s = display.styles;

	// The initial height of the Bulletin is zero and it starts hidden so that
	// it's not displayed until the first layout.
	SetVisible(false);

	bg = AddChild(new FillBox(580, 100, s.announcementBg));

	clickBox = AddChild(new ClickRegion(display, Vec2(580, 100)));
	clickBox->GetClickedSignal().connect(std::bind(&Announcement::OnClick,
		ann.get()));

	labelLbl = AddChild(new Label(420, ann->GetLabel(),
		s.announcementHeadFont, s.announcementHeadFg));
	labelLbl->SetPos(120, 20);

	auto icon = AddChild(ann->CreateIcon(display));
	icon->SetPos(40, 20);
	icon->SetSize(60, 60);

	//TODO: Player avatar.

	contentGrid = AddChild(new FlexGrid(display));
	contentGrid->SetMargin(3, 0);
	contentGrid->SetPos(120, 60);
	contentGrid->SetFixedWidth(420);
	ann->CreateContents(display, contentGrid.get());
}

void BulletinBoard::Bulletin::Layout()
{
	SetVisible(true);

	Vec3 gridSize = contentGrid->Measure();
	Vec2 fullSize{ GetSize().x, 60 + gridSize.y + 10 };
	if (GetSize() != fullSize) {
		SetSize(fullSize);
		bg->SetSize(fullSize);
		clickBox->SetSize(fullSize);
		board->OnBulletinSizeUpdated();
	}
}

//}}} Bulletin

}  // namespace HoverScript
}  // namespace Client
