
// HudDecor.cpp
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

#include "HudDecor.h"

namespace HoverRace {
namespace Display {

/**
 * Constructor.
 * @param display The display child elements will be attached to.
 */
HudDecor::HudDecor(Display &display) :
	SUPER(display),
	player(nullptr), track(), hudScale(1, 1)
{
}

void HudDecor::FireModelUpdate(int prop)
{
	switch (prop) {
		case Container::Props::SIZE:
			sizeChangedSignal(GetSize());
			break;
	}
	SUPER::FireModelUpdate(prop);
}

/**
 * Change the target player for this HUD element.
 * @param player The player (may be @c nullptr).
 */
void HudDecor::SetPlayer(std::shared_ptr<Player::Player> player)
{
	if (this->player != player) {
		this->player = std::move(player);
		FireModelUpdate(Props::PLAYER);
	}
}

/**
 * Change the track currently being played.
 * @param track The track (may not be @c nullptr).
 */
void HudDecor::SetTrack(std::shared_ptr<Model::Track> track)
{
	if (this->track != track) {
		this->track = track;
		FireModelUpdate(Props::TRACK);
	}
}

/**
 * Sets HUD scale factor.
 * @param hudScale The HUD scale.
 */
void HudDecor::SetHudScale(const Vec2 &hudScale)
{
	if (this->hudScale != hudScale) {
		this->hudScale = hudScale;
		FireModelUpdate(Props::HUD_SCALE);
		OnHudRescaled(hudScale);
	}
}

}  // namespace Display
}  // namespace HoverRace
