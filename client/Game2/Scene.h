
// Scene.h
// Base class for scenes.
//
// Copyright (c) 2010, 2013 Michael Imamura.
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

#include "../../engine/Util/OS.h"

namespace HoverRace {
	namespace Control {
		class InputEventController;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Scenes are analogous to fullscreen windows that are layered on top of each
 * other.  Each scene maintains its own state and handles its own rendering.
 * Scenes navigate to other scenes via GameDirector.
 * @see GameDirector
 * @author Michael Imamura
 */
class Scene
{
	public:
		struct Phase
		{
			enum phase_t
			{
				INITIALIZING,  ///< Scene has been created and is waiting to start.
				STARTING,  ///< Transitioning into the scene.
				RUNNING,  ///< Started and running normally.
				STOPPING,  ///< Transitioning away from the scene.
				STOPPED,  ///< The scene is ready to be destroyed.
			};
		};

	public:
		Scene(const std::string &name="Unnamed Scene");
		virtual ~Scene() { }

	public:
		/**
		 * Retrieve the name of the scene.
		 * This is for debugging and logging purposes; it is otherwise not
		 * intended to be user-visible (and thus should not be translated).
		 * @return The name (may be empty).
		 */
		const std::string &GetName() const { return name; }

		/**
		 * Configure the controller mappings.
		 * This is called whenever the scene becomes the foreground scene
		 * or the controller config needs to be rebuilt.
		 * @param controller The current controller being used.  It can be assumed
		 *                   that the controller's action mappings have been cleared.
		 */
		virtual void AttachController(Control::InputEventController &controller) = 0;

		/**
		 * Remove the controller mappings.
		 * @param controller The current controller being used.  It can be assumed
		 *                   that the controller's action mappings have been cleared.
		 */
		virtual void DetachController(Control::InputEventController &controller) = 0;

		/**
		 * Determine if the mouse cursor is enabled for this scene.
		 * @return @c true if the cursor should be shown,
		 *         @c false if it should be hidden.
		 */
		virtual bool IsMouseCursorEnabled() const = 0;

	public:
		Phase::phase_t GetPhase() const { return phase; }
		bool SetPhase(Phase::phase_t phase);

	protected:
		/**
		 * Set the maximum duration of the starting or stopping phases.
		 *
		 * Subclasses should call this function in the constructor or the first
		 * tick in order to set how long the starting phase will last.  The
		 * stopping phase always lasts as long as the starting phase.
		 *
		 * If set to zero (the default), then the starting and stopping phases
		 * will be skipped.
		 *
		 * If set to non-zero, then the OnTransition() function will be called
		 * during Advance() so subclasses can customize the animation.
		 * @param ms The duration (may be zero to disable starting and
		 *           stopping phases.
		 */
		void SetTransitionDuration(Util::OS::timestamp_t ms)
		{
			transitionDuration = static_cast<double>(ms);
		}

		/**
		 * Calculate how much time we've spent in the current phase.
		 * @param curTime The current (or simulated) timestamp.
		 * @return The duration. If @p curTime is before the start of the
		 *         phase, then zero is returned.
		 */
		Util::OS::timestamp_t GetPhaseDuration(Util::OS::timestamp_t curTime=Util::OS::Time())
		{
			return (curTime >= phaseTs) ?
				Util::OS::TimeDiff(curTime, phaseTs) :
				0;
		}

		/**
		 * Return how long the starting phase lasted.
		 * This is useful for making the @c STOPPING phase have the same duration as
		 * the @c STARTING phase, especially if the @c STARTING phase was
		 * interrupted.
		 * If the current phase is the @c STARTING phase, then this returns
		 * the current time in the phase so far.
		 * @return The duration of the phase.
		 */
		Util::OS::timestamp_t GetStartingPhaseTime()
		{
			return (phase == Phase::STARTING) ? GetPhaseDuration() : startingPhaseTime;
		}

	protected:
		/**
		 * Fired immediately after entering a new phase.
		 * @param oldPhase The previous phase.
		 * @see GetPhase()
		 */
		virtual void OnPhaseChanged(Phase::phase_t oldPhase) { }

		/**
		 * Fired during the starting and stopping phases, if
		 * SetTransitionDuration() was set.
		 * @param progress The animation progress (during the @c STARTING phase
		 *                 this goes from 0.0 to 1.0, and in reverse for the
		 *                 @c STOPPING phase).
		 */
		virtual void OnTransition(double progress) { }

	public:
		virtual void Advance(Util::OS::timestamp_t tick);
		virtual void PrepareRender() { }
		virtual void Render() = 0;

	private:
		std::string name;
		Phase::phase_t phase;
		double transitionDuration;
		Util::OS::timestamp_t phaseTs;  ///< Timestamp of when current phase was started.
		Util::OS::timestamp_t startingPhaseTime;
};
typedef std::shared_ptr<Scene> ScenePtr;

}  // namespace HoverScript
}  // namespace Client
