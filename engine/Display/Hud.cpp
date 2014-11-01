
// Hud.cpp
//
// Copyright (c) 2013, 2014 Michael Imamura.
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

#include "../Player/Player.h"

#include "Hud.h"

namespace HoverRace {
namespace Display {

namespace {
	const double HUD_MARGIN = 20;  ///< Margin around HUD edges.
	const double DECOR_MARGIN = 5;  ///< Space between decor elements.
}

//{{{ HudChild ////////////////////////////////////////////////////////////////

Hud::HudChild::HudChild(std::shared_ptr<HudDecor> decor) :
	decor(std::move(decor))
{
}

Hud::HudChild::HudChild(HudChild &&other) :
	decor(std::move(other.decor)),
	sizeChangedConn(other.sizeChangedConn.release())
{
}

Hud::HudChild &Hud::HudChild::operator=(HudChild &&other)
{
	decor = std::move(other.decor);
	sizeChangedConn = other.sizeChangedConn.release();
	return *this;
}

//}}} HudChild

/**
 * Constructor.
 * @param display The display child elements will be attached to.
 * @param player The target player this HUD is viewing.
 * @param track The track being played.
 * @param size The size of the container.
 * @param clip Enable (default) or disable clipping of child widgets.
 * @param layoutFlags Optional layout flags.
 */
Hud::Hud(Display &display, std::shared_ptr<Player::Player> player,
	std::shared_ptr<Model::Track> track,
	uiLayoutFlags_t layoutFlags) :
	SUPER(display, Vec2(1280, 720), false, layoutFlags),
	track(std::move(track)), player(std::move(player)),
	cell(HudCell::FILL)
{
	displayConfigChangedConn =
		display.GetDisplayConfigChangedSignal().connect(
			std::bind(&Hud::OnScreenSizeChanged, this));

	OnScreenSizeChanged();
}

/**
 * Change the player being targeted by the HUD.
 * @param player The target player (may be @c nullptr).
 */
void Hud::SetPlayer(std::shared_ptr<Player::Player> player)
{
	if (this->player != player) {
		this->player = player;
		ForEachHudChild([&](std::shared_ptr<HudDecor> &child) {
			child->SetPlayer(player);
		});
		FireModelUpdate(Props::PLAYER);
	}
}

/**
 * Change the track being played.
 * @param track The track (may not be @c nullptr).
 */
void Hud::SetTrack(std::shared_ptr<Model::Track> track)
{
	if (this->track != track) {
		this->track = track;
		ForEachHudChild([&](std::shared_ptr<HudDecor> &child) {
			child->SetTrack(track);
		});
		FireModelUpdate(Props::TRACK);
	}
}

/**
 * Set the subsection of the screen where this HUD should be rendered.
 */
void Hud::SetCell(HudCell cell)
{
	if (this->cell != cell) {
		this->cell = cell;
		OnScreenSizeChanged();
	}
}

/**
 * Fired when the screen size changes.
 */
void Hud::OnScreenSizeChanged()
{
	
	auto size = display.GetUiScreenSize();
	auto pos = Vec2{0, 0};

	// Adjust size for splitscreen viewports.
	switch (cell) {
		case HudCell::FILL:
			// No size adjustment.
			break;

		case HudCell::N:
		case HudCell::S:
			size.y /= 2.0;
			break;

		case HudCell::NW:
		case HudCell::NE:
		case HudCell::SE:
		case HudCell::SW:
			size.x /= 2.0;
			size.y /= 2.0;
			break;

		default:
			throw UnimplementedExn("Hud::OnScreenSizeChanged: Cell type: " +
				boost::lexical_cast<std::string>(cell));
	}
	SetSize(size);

	// Adjust pos for splitscreen viewports.
	switch (cell) {
		case HudCell::FILL:
		case HudCell::N:
		case HudCell::NW:
			// No pos adjustment.
			break;
		case HudCell::S:
		case HudCell::SW:
			pos.y = size.y;
			break;
		case HudCell::NE:
			pos.x = size.x;
			break;
		case HudCell::SE:
			pos = size;
			break;
		default:
			throw UnimplementedExn("Hud::OnScreenSizeChanged: Cell type: " +
				boost::lexical_cast<std::string>(cell));
	}
	SetPos(pos);

	RequestLayout();
}

void Hud::LayoutStacked(HudAlignment::type align,
                        double startX, double startY,
                        double scaleX, double scaleY)
{
	double x = startX;
	double y = startY;
	for (auto &child : hudChildren[align]) {
		auto &elem = child.decor;
		elem->SetPos(startX, y);
		Vec3 sz = elem->Measure();
		x += scaleX * (sz.x + DECOR_MARGIN);
		y += scaleY * (sz.y + DECOR_MARGIN);
	}
}

void Hud::LayoutCorner(HudAlignment::type alignCorner,
                       HudAlignment::type alignH, HudAlignment::type alignV,
                       double startX, double startY,
                       double scaleX, double scaleY)
{
	double x = startX, y = startY;
	double maxHeightH = 0;

	auto &cornerElems = hudChildren[alignCorner];
	if (!cornerElems.empty()) {
		auto &elem = cornerElems.back().decor;
		elem->SetPos(x, y);
		Vec3 sz = elem->Measure();
		maxHeightH = sz.y;
		x += scaleX * (sz.x + DECOR_MARGIN);
	}

	// Layout the horizontal elems.
	for (auto &child : hudChildren[alignH]) {
		auto &elem = child.decor;
		elem->SetPos(x, y);
		Vec3 sz = elem->Measure();
		if (sz.y > maxHeightH) {
			maxHeightH  = sz.y;
		}
		x += scaleX * (sz.x + DECOR_MARGIN);
	}

	// Layout the vertical elems.
	y = startY + (scaleY * (maxHeightH + DECOR_MARGIN));
	x = startX;
	for (auto &child : hudChildren[alignV]) {
		auto &elem = child.decor;
		elem->SetPos(x, y);
		Vec3 sz = elem->Measure();
		y += scaleY * (sz.y + DECOR_MARGIN);
	}
}

void Hud::Layout()
{
	SUPER::Layout();

	const Vec2 &hudSize = GetSize();

	const double midX = hudSize.x / 2.0;
	const double midY = hudSize.y / 2.0;
	const double divY = hudSize.y / 3.0;

	const double north = HUD_MARGIN;
	const double east = hudSize.x - HUD_MARGIN;
	const double south = hudSize.y - HUD_MARGIN;
	const double west = HUD_MARGIN;

	//TODO: Only redo the corners or sides that changed.

	LayoutCorner(HudAlignment::NW, HudAlignment::NNW, HudAlignment::WNW,
		west, north, 1, 1);
	LayoutCorner(HudAlignment::NE, HudAlignment::NNE, HudAlignment::ENE,
		east, north, -1, 1);
	LayoutCorner(HudAlignment::SE, HudAlignment::SSE, HudAlignment::ESE,
		east, south, -1, -1);
	LayoutCorner(HudAlignment::SW, HudAlignment::SSE, HudAlignment::WSW,
		west, south, 1, -1);

	LayoutStacked(HudAlignment::N, midX, north, 0, 1);
	LayoutStacked(HudAlignment::E, east, midY, -1, 0);
	LayoutStacked(HudAlignment::S, midX, south, 0, -1);
	LayoutStacked(HudAlignment::W, west, midY, 1, 0);

	LayoutStacked(HudAlignment::ABOVE, midX, divY, 0, -1);
	LayoutStacked(HudAlignment::BELOW, midX, divY + divY, 0, 1);
}

void Hud::Advance(Util::OS::timestamp_t tick)
{
	ForEachHudChild([&](std::shared_ptr<HudDecor> &child) {
		child->Advance(tick);
	});
}

}  // namespace Display
}  // namespace HoverRace
