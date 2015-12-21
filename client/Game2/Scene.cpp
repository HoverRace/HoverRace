
// Scene.cpp
//
// Copyright (c) 2013, 2015 Michael Imamura.
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

#include "../../engine/Util/Log.h"

#include "Scene.h"

#ifdef min
#	undef min
#endif

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

/**
 * Constructor.
 * @param name The name of the scene.  See Scene::GetName.
 */
Scene::Scene(const std::string &name) :
	name(name), prevTick(0),
	phase(Phase::INITIALIZING), phaseTransitionDuration(0),
	phaseTs(0), startingPhaseTime(0),
	state(State::INITIALIZING), stateTransitionDuration(0),
	stateTs(0), stateTransitionVelocity(1), statePosition(0),
	rootProfiler(std::make_shared<Profiler>("ROOT")),
	advanceProfiler(rootProfiler->AddSub("advance")),
	prepareProfiler(rootProfiler->AddSub("prepare")),
	renderProfiler(rootProfiler->AddSub("render"))
{
}

void Scene::AttachInput(Control::InputEventController &controller)
{
	AttachController(controller);
}

void Scene::DetachInput(Control::InputEventController &controller)
{
	DetachController(controller);
}

/**
 * Switch to another phase.
 * @warning Scenes should not switch to the @c STOPPING phase themselves.
 *          Instead, they should call GameDirector::PopScene or
 *          GameDirector::ReplaceScene.
 * @param phase The new phase.
 * @return @c true if the phase change was successful, @c false otherwise.
 */
bool Scene::SetPhase(Phase phase)
{
	if (this->phase != phase) {
		// Can't go back to the initial phase.
		if (phase == Phase::INITIALIZING) return false;

		// Once shutdown has started, it can't be stopped.
		if (this->phase == Phase::STOPPING && phase != Phase::STOPPED) return false;

		if (this->phase == Phase::STARTING) {
			// Save the starting phase duration in case the subclass wants to
			// use it later.
			startingPhaseTime = GetPhaseDuration();
		}
		else if (this->phase == Phase::INITIALIZING) {
			// Start in the BACKGROUND state.
			SetState(State::BACKGROUND);
		}

		phaseTs = OS::Time();
		Phase oldPhase = this->phase;
		this->phase = phase;

		OnPhaseChanged(oldPhase);

		return true;
	}
	else {
		return false;
	}
}

/**
 * Signal that the scene is moving into the foreground.
 * @return @c true if the state changed.
 */
bool Scene::MoveToForeground()
{
	if (state == State::BACKGROUND || state == State::LOWERING) {
		return SetState(State::RAISING);
	}
	return false;
}

/**
 * Signal that the scene is moving into the background.
 * @return @c true if the state changed.
 */
bool Scene::MoveToBackground()
{
	if (state == State::FOREGROUND || state == State::RAISING) {
		return SetState(State::LOWERING);
	}
	return false;
}

bool Scene::SetState(State state)
{
	if (this->state != state) {

		stateTs = OS::Time();
		State oldState = this->state;
		this->state = state;

		OnStateChanged(oldState);

		return true;
	}
	else {
		return false;
	}
}

void Scene::Advance(Util::OS::timestamp_t tick)
{
	if (prevTick == 0) {
		prevTick = tick;
	}

	// Handle the starting and stopping animation.
	switch (GetPhase()) {
		case Phase::STARTING: {
			Util::OS::timestamp_t duration = GetPhaseDuration(tick);
			if (duration >= phaseTransitionDuration) {
				OnPhaseTransition(1.0);
				SetPhase(Phase::RUNNING);
			}
			else {
				OnPhaseTransition(static_cast<double>(duration) / phaseTransitionDuration);
			}
			break;
		}

		case Phase::STOPPING: {
			Util::OS::timestamp_t duration = GetPhaseDuration(tick);
			Util::OS::timestamp_t startingDuration = GetStartingPhaseTime();
			if (duration >= startingDuration) {
				OnPhaseTransition(0.0);
				SetPhase(Phase::STOPPED);
			}
			else {
				OnPhaseTransition(static_cast<double>(startingDuration - duration) / phaseTransitionDuration);
			}
			break;
		}

		default:
			break;
	}

	// Handle the raising and lowering animation.
	switch (GetState()) {
		case State::RAISING: {
			double elapsed = static_cast<double>(TimeSincePrevTick(tick));
			statePosition += elapsed * stateTransitionVelocity;
			if (statePosition >= 1.0) {
				statePosition = 1.0;
				OnStateTransition(1.0);
				SetState(State::FOREGROUND);
			}
			else {
				OnStateTransition(statePosition);
			}
			break;
		}

		case State::LOWERING: {
			double elapsed = static_cast<double>(TimeSincePrevTick(tick));
			statePosition -= elapsed * stateTransitionVelocity;
			if (statePosition <= 0) {
				statePosition = 0;
				OnStateTransition(0);
				SetState(State::BACKGROUND);
			}
			else {
				OnStateTransition(statePosition);
			}
			break;
		}

		default:
			break;
	}

	prevTick = tick;
}

}  // namespace HoverScript
}  // namespace Client
