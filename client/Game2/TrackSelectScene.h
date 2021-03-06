
// TrackSelectScene.h
//
// Copyright (c) 2013-2015 Michael Imamura.
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

#include "../../engine/Display/PickList.h"
#include "../../engine/Display/Res.h"
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
		class FlexGrid;
		class Label;
		class Picture;
		class RuleLine;
		class Texture;
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
	using SUPER = DialogScene;

public:
	TrackSelectScene(Display::Display &display, GameDirector &director,
		std::shared_ptr<const Rulebook> rulebook);
	virtual ~TrackSelectScene();

private:
	void OnTrackSelected(std::shared_ptr<Model::TrackEntry> entry);
	void OnTrackChanged();
	void OnReady();

protected:
	void OnPhaseTransition(double progress) override;

public:
	using okSignal_t = boost::signals2::signal<void(std::shared_ptr<Rules>, std::shared_ptr<Display::Res<Display::Texture>>)>;
	okSignal_t &GetOkSignal() { return okSignal; }

	using cancelSignal_t = boost::signals2::signal<void()>;
	cancelSignal_t &GetCancelSignal() { return cancelSignal; }

private:
	bool trackSelected;  ///< Are we exiting because a track was selected?
	std::shared_ptr<Rules> rules;

	Model::TrackList trackList;
	std::shared_ptr<Model::TrackEntry> selectedTrack;

	std::shared_ptr<Display::FlexGrid> subtitleGrid;
	std::shared_ptr<Display::Label> rulebookLbl;
	std::shared_ptr<Display::Label> rulebookDescLbl;
	std::shared_ptr<Display::RuleLine> subtitleRule;
	std::shared_ptr<Display::Container> trackPanel;
	std::shared_ptr<Display::PickList<
		std::shared_ptr<Model::TrackEntry>>> trackPick;

	std::shared_ptr<Display::Container> selTrackPanel;
	std::shared_ptr<Display::Picture> trackPic;
	std::shared_ptr<Display::Res<Display::Texture>> mapTexture;

	std::shared_ptr<Display::FlexGrid> trackMetaGrid;
	std::shared_ptr<Display::Label> trackNameLbl;
	std::shared_ptr<Display::Label> trackDescLbl;

	std::shared_ptr<Display::Button> readyBtn;

	okSignal_t okSignal;
	cancelSignal_t cancelSignal;
};

}  // namespace Client
}  // namespace HoverRace
