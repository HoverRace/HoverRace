
// Scene.h
//
// Copyright (c) 2010, 2013-2015 Michael Imamura.
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
#include "../../engine/Util/Profiler.h"

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
	enum class Phase
	{
		INITIALIZING,  ///< Scene has been created and is waiting to start.
		STARTING,  ///< Transitioning into the scene.
		RUNNING,  ///< Started and running normally.
		STOPPING,  ///< Transitioning away from the scene.
		STOPPED,  ///< The scene is ready to be destroyed.
	};
	enum class State
	{
		INITIALIZING,  ///< Scene has been created and is waiting to start.
		BACKGROUND,  ///< Scene is in the background.
		RAISING,  ///< Has focus, transitioning to the foreground.
		FOREGROUND,  /// Has focus, in foreground.
		LOWERING,  ///< Lost focus, transitioning to the background.
	};

public:
	struct ConnList
	{
	public:
		~ConnList()
		{
			// Disconnect the connections in reverse order of add.
			while (!conns.empty()) {
				conns.pop_back();
			}
		}

		ConnList &operator<<(boost::signals2::connection conn)
		{
			conns.emplace_back(new boost::signals2::scoped_connection(conn));
			return *this;
		}
	private:
		std::list<std::unique_ptr<boost::signals2::scoped_connection>> conns;
	};

public:
	Scene(const std::string &name = "Unnamed Scene");
	virtual ~Scene() { }

	Scene &operator=(const Scene&) = delete;

public:
	/**
	 * Retrieve the name of the scene.
	 * This is for debugging and logging purposes; it is otherwise not
	 * intended to be user-visible (and thus should not be translated).
	 * @return The name (may be empty).
	 */
	const std::string &GetName() const { return name; }

	/**
	 * Output a stream of debug information describing this scene.
	 * The output text may include newlines.
	 * @param [in,out] oss The output stream to write to.
	 * @return The same output stream as was passed in.
	 */
	virtual std::ostream &OutputDebugText(std::ostream &oss) const { return oss; }

	/**
	 * Configure the controller mappings.
	 * This is called whenever the scene becomes the foreground scene
	 * or the controller config needs to be rebuilt.
	 * @param controller The current controller being used.  It can be assumed
	 *                   that the controller's action mappings have been
	 *                   cleared.
	 */
	virtual void AttachController(Control::InputEventController &controller) = 0;

	/**
	 * Remove the controller mappings.
	 * @param controller The current controller being used.  It can be assumed
	 *                   that the controller's action mappings have been
	 *                   cleared.
	 */
	virtual void DetachController(Control::InputEventController &controller) = 0;

	/**
	 * Called when the scene is actually pushed to the stage.
	 *
	 * In terms of phases, this is called just <i>before</i> the scene
	 * enters the @c STARTING phase, so the scene will be still in the
	 * @c INITIALIZING phase.
	 *
	 * This is useful for handling anything that needs to be done just
	 * before the first frame is rendered, but after the constructor has
	 * run.
	 *
	 * An example is pushing another scene on top of this scene.  This
	 * can't be done in the constructor since this scene hasn't been pushed
	 * yet (so the scenes would be out-of-order on the stage).
	 */
	virtual void OnScenePushed() { }

	/**
	 * Determine if the mouse cursor is enabled for this scene.
	 * @return @c true if the cursor should be shown,
	 *         @c false if it should be hidden.
	 */
	virtual bool IsMouseCursorEnabled() const = 0;

private:
	Util::OS::timestamp_t TimeSincePrevTick(Util::OS::timestamp_t tick)
	{
		return (tick >= prevTick) ?
			Util::OS::TimeDiff(tick, prevTick) :
			0;
	}

public:
	Phase GetPhase() const { return phase; }
	bool SetPhase(Phase phase);

	State GetState() const { return state; }
	bool MoveToForeground();
	bool MoveToBackground();
private:
	bool SetState(State state);

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
	 * If set to non-zero, then the OnPhaseTransition() function will be
	 * called during Advance() so subclasses can customize the animation.
	 * @param ms The duration (may be zero to disable starting and
	 *           stopping phases).
	 */
	void SetPhaseTransitionDuration(Util::OS::timestamp_t ms)
	{
		phaseTransitionDuration = static_cast<double>(ms);
	}

	/**
	 * Calculate how much time we've spent in the current phase.
	 * @param curTime The current (or simulated) timestamp.
	 * @return The duration. If @p curTime is before the start of the
	 *         phase, then zero is returned.
	 */
	Util::OS::timestamp_t GetPhaseDuration(
		Util::OS::timestamp_t curTime = Util::OS::Time())
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
		return (phase == Phase::STARTING) ?
			GetPhaseDuration() :
			startingPhaseTime;
	}

	/**
	 * Calculate how much time we've spent in the current state.
	 * @param curTime The current (or simulated) timestamp.
	 * @return The duration. If @p curTime is before the start of the
	 *         state, then zero is returned.
	 */
	Util::OS::timestamp_t GetStateDuration(
		Util::OS::timestamp_t curTime = Util::OS::Time())
	{
		return (curTime >= stateTs) ?
			Util::OS::TimeDiff(curTime, stateTs) :
			0;
	}

	/**
	 * Set the maximum duration of the raising or lowering state.
	 *
	 * Subclasses should call this function in the constructor or the first
	 * tick in order to set how long the raising state will last.  The
	 * lowering state always lasts as long as the raising state.
	 *
	 * If set to zero (the default), then the raising and lowering states
	 * will be skipped.
	 *
	 * If set to non-zero, then the OnStateTransition() function will be
	 * called during Advance() so subclasses can customize the animation.
	 * @param ms The duration (may be zero to disable raising and
	 *           lowering states).
	 */
	void SetStateTransitionDuration(Util::OS::timestamp_t ms)
	{
		stateTransitionDuration = ms;
		stateTransitionVelocity =
			(ms == 0) ? 1.0 : (1.0 / static_cast<double>(ms));
	}

protected:
	/**
	 * Fired immediately after entering a new phase.
	 * @param oldPhase The previous phase.
	 * @see GetPhase()
	 */
	virtual void OnPhaseChanged(Phase oldPhase) { HR_UNUSED(oldPhase); }

	/**
	 * Fired immediately after entering a new state.
	 * @param oldState The previous state.
	 * @see GetState()
	 */
	virtual void OnStateChanged(State oldState) { HR_UNUSED(oldState); }

	/**
	 * Fired during the starting and stopping phases, if
	 * SetPhaseTransitionDuration() was set.
	 * @param progress The animation progress (during the @c STARTING phase
	 *                 this goes from 0.0 to 1.0, and in reverse for the
	 *                 @c STOPPING phase).
	 */
	virtual void OnPhaseTransition(double progress) { HR_UNUSED(progress); }

	/**
	 * Fired during the raising and lowering states, if
	 * SetStateTransitionDuration() was set.
	 * @param progress The animation progress (during the @c RAISING phase
	 *                 this goes from 0.0 to 1.0, and in reverse for the
	 *                 @c LOWERING phase).
	 */
	virtual void OnStateTransition(double progress) { HR_UNUSED(progress); }

public:
	void AdvanceScene(Util::OS::timestamp_t tick)
	{
		Util::Profiler::Sampler rootSampler(*rootProfiler);
		Util::Profiler::Sampler sampler(*advanceProfiler);
		Advance(tick);
	}

	void PrepareScene()
	{
		Util::Profiler::Sampler rootSampler(*rootProfiler);
		Util::Profiler::Sampler sampler(*prepareProfiler);
		PrepareRender();
	}

	void RenderScene()
	{
		Util::Profiler::Sampler rootSampler(*rootProfiler);
		Util::Profiler::Sampler sampler(*renderProfiler);
		Render();
	}

protected:
	virtual void Advance(Util::OS::timestamp_t tick);
	virtual void PrepareRender() { }
	virtual void Render() = 0;

private:
	std::string name;
	Util::OS::timestamp_t prevTick;
	Phase phase;
	double phaseTransitionDuration;
	Util::OS::timestamp_t phaseTs;  ///< When current phase was started.
	Util::OS::timestamp_t startingPhaseTime;
	State state;
	Util::OS::timestamp_t stateTransitionDuration;
	Util::OS::timestamp_t stateTs;  ///< When current state was started.
	double stateTransitionVelocity;
	double statePosition;
protected:
	std::shared_ptr<Util::Profiler> rootProfiler;
	std::shared_ptr<Util::Profiler> advanceProfiler;
	std::shared_ptr<Util::Profiler> prepareProfiler;
	std::shared_ptr<Util::Profiler> renderProfiler;
};

}  // namespace HoverScript
}  // namespace Client
