
// HudDecor.h
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

#pragma once

#include "../Util/OS.h"

#include "Container.h"

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
	namespace Display {
		class Display;
	}
	namespace Player {
		class Player;
	}
	namespace Model {
		class Track;
	}
}

namespace HoverRace {
namespace Display {

/**
 * Base view model for elements of the heads-up display.
 * @author Michael Imamura
 * @see Hud
 */
class MR_DllDeclare HudDecor : public Container
{
	typedef Container SUPER;

public:
	struct Props
	{
		enum {
			PLAYER = SUPER::Props::NEXT_,
			TRACK,
			HUD_SCALE,
			NEXT_,  ///< First index for subclasses.
		};
	};

public:
	HudDecor(Display &display);
	virtual ~HudDecor() { }

public:
	typedef boost::signals2::signal<void(const Vec2&)> sizeChangedSignal_t;
	sizeChangedSignal_t &GetSizeChangedSignal() { return sizeChangedSignal; }

protected:
	void FireModelUpdate(int prop) override;

	virtual void OnHudRescaled(const Vec2 &hudScale) { }

public:
	virtual void Advance(Util::OS::timestamp_t) { };

public:
	Player::Player *GetPlayer() const { return player.get(); }
	std::shared_ptr<Player::Player> SharePlayer() const { return player; }
	void SetPlayer(std::shared_ptr<Player::Player> player);

	Model::Track *GetTrack() const { return track.get(); }
	std::shared_ptr<Model::Track> ShareTrack() const { return track; }
	void SetTrack(std::shared_ptr<Model::Track> track);

	const Vec2 &GetHudScale() const { return hudScale; }
	void SetHudScale(const Vec2 &hudScale);

private:
	std::shared_ptr<Player::Player> player;
	std::shared_ptr<Model::Track> track;
	Vec2 hudScale;
	sizeChangedSignal_t sizeChangedSignal;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
