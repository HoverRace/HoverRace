
// LoadingScene.h
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

#include "GameDirector.h"

#include "FormScene.h"

namespace HoverRace {
	namespace Display {
		class ScreenFade;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Base class for "Please Wait" loading scenes.
 *
 * These scenes provide a way to wait for the previous scene stack to finish
 * unloading resources then allow the new scene to load.
 *
 * @author Michael Imamura
 */
class LoadingScene : public FormScene
{
	typedef FormScene SUPER;
	public:
		LoadingScene(Display::Display &display, GameDirector &director,
			const std::string &name="Loading");
		virtual ~LoadingScene();

	public:
		typedef boost::signals2::signal<void()> finishedLoadingSignal_t;
		/**
		 * Fired when all resources have been loaded and the loading scene
		 * is shutting down.
		 * @return The finished loading signal.
		 */ 
		finishedLoadingSignal_t &GetFinishedLoadingSignal()
		{
			return finishedLoadingSignal;
		}

	protected:
		void OnPhaseChanged(Phase oldPhase) override;

	public:
		void OnPhaseTransition(double progress) override;
		void PrepareRender() override;
		void Render() override;

	private:
		GameDirector &director;
		bool loading;
		std::unique_ptr<Display::ScreenFade> fader;

		finishedLoadingSignal_t finishedLoadingSignal;
};

}  // namespace Client
}  // namespace HoverRace
