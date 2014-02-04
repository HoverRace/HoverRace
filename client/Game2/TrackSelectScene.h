
// TrackSelectScene.h
//
// Copyright (c) 2013 Michael Imamura.
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

#include "../../engine/Model/TrackList.h"
#include "GameDirector.h"

#include "DialogScene.h"

namespace HoverRace {
	namespace Client {
		class Rulebook;
	}
	namespace Display {
		class Container;
		class Display;
		class Label;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Select the track for a new game.
 * @author Michael Imamura
 */
class TrackSelectScene : public DialogScene
{
	typedef DialogScene SUPER;
	public:
		TrackSelectScene(Display::Display &display, GameDirector &director,
			std::shared_ptr<const Rulebook> rulebook);
		virtual ~TrackSelectScene();

	private:
		void OnTrackSelected(Model::TrackEntryPtr entry);

	protected:
		virtual void OnPhaseTransition(double progress);


	public:
		typedef boost::signals2::signal<void(std::shared_ptr<Rules>)> okSignal_t;
		okSignal_t &GetOkSignal() { return okSignal; }

		typedef boost::signals2::signal<void()> cancelSignal_t;
		cancelSignal_t &GetCancelSignal() { return cancelSignal; }

	private:
		std::shared_ptr<Rules> rules;

		Model::TrackList trackList;

		std::shared_ptr<Display::Label> rulebookLbl;
		std::shared_ptr<Display::Container> trackPanel;

		okSignal_t okSignal;
		cancelSignal_t cancelSignal;
};

}  // namespace Client
}  // namespace HoverRace
