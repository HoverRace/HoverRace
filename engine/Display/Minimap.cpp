
// Minimap.cpp
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

#include "../../engine/MainCharacter/MainCharacter.h"
#include "../../engine/Player/Player.h"
#include "../../engine/Model/Track.h"
#include "../../engine/Util/Log.h"
#include "Picture.h"
#include "SymbolIcon.h"

#include "Minimap.h"

namespace HoverRace {
namespace Display {

namespace {
const int ICON_SYMBOL = 0xf111;  // circle
const double MAP_WIDTH = 200;
const double MAP_HEIGHT = 200;
}  // namespace

/**
 * Constructor.
 * @param display The display child elements will be attached to.
 */
Minimap::Minimap(Display &display) :
	SUPER(display), rescale(1.0), mapScale(0, 0), mapPic(), playerIcon()
{
	typedef UiViewModel::Alignment Alignment;

	SetSize(MAP_WIDTH, MAP_HEIGHT);
	SetClip(false);
	
	mapPic = AddChild(new Picture(std::shared_ptr<Res<Texture>>(),
		MAP_WIDTH, MAP_HEIGHT));

	playerIcon = AddChild(new SymbolIcon(10, 10, ICON_SYMBOL));
	playerIcon->SetAlignment(Alignment::CENTER);
}

void Minimap::OnHudRescaled(const Vec2 &hudScale)
{
	// Maintain the map aspect ratio.
	rescale = std::min(hudScale.x, hudScale.y);
	
	Vec2 sz{MAP_WIDTH * rescale, MAP_HEIGHT * rescale};
	SetSize(sz);
	mapPic->SetSize(sz);

	RequestLayout();
}

void Minimap::FireModelUpdate(int prop)
{
	switch (prop) {
		case HudDecor::Props::TRACK: {
			auto track = GetTrack();
			if (track) {
				mapPic->SetTexture(track->GetMap());
				auto &trackSize = track->GetSize();
				mapScale.x = MAP_WIDTH / trackSize.x;
				mapScale.y = MAP_HEIGHT / trackSize.y;
			}
			else {
				mapPic->SetTexture(std::shared_ptr<Res<Texture>>());
				mapScale.x = 0;
				mapScale.y = 0;
			}
			break;
		}
	}
	SUPER::FireModelUpdate(prop);
}

void Minimap::Advance(Util::OS::timestamp_t)
{
	auto track = GetTrack();
	if (!track) return;

	auto &trackOffset = track->GetOffset();

	//TODO: Update all players, not just the current player.
	auto player = GetPlayer();
	if (!player) return;
	auto mchar = player->GetMainCharacter();
	if (!mchar) return;

	auto scale = mapScale * rescale;

	Vec2 pos{
		static_cast<double>(mchar->mPosition.mX),
		static_cast<double>(mchar->mPosition.mY)
	};
	playerIcon->SetColor(player->GetProfile()->GetPrimaryColor());
	playerIcon->SetPos(
		(pos.x - trackOffset.x) * scale.x,
		GetSize().y - ((pos.y - trackOffset.y) * scale.y));
}

}  // namespace Display
}  // namespace HoverRace
