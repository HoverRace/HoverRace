
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
				INITIALIZED,  ///< Scene has been created and is waiting to start.
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

	public:
		Phase::phase_t GetPhase() const { return phase; }
		bool SetPhase(Phase::phase_t phase);

		/**
		 * Calculate how much time we've spent in the current phase.
		 * @param curTime The current (or simulated) timestamp.
		 */
		Util::OS::timestamp_t GetPhaseDuration(Util::OS::timestamp_t curTime=Util::OS::Time())
		{
			return Util::OS::TimeDiff(curTime, phaseTs);
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

		virtual void OnPhaseChanged(Phase::phase_t oldPhase);

	public:
		virtual void Advance(Util::OS::timestamp_t tick) = 0;
		virtual void PrepareRender() { }
		virtual void Render() = 0;

	private:
		std::string name;
		Phase::phase_t phase;
		Util::OS::timestamp_t phaseTs;  ///< Timestamp of when current phase was started.
		Util::OS::timestamp_t startingPhaseTime;
};
typedef std::shared_ptr<Scene> ScenePtr;

}  // namespace HoverScript
}  // namespace Client
