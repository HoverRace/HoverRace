
// Scene.cpp
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

#include "StdAfx.h"

#include "Scene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

/**
 * Constructor.
 * @param name The name of the scene.  See Scene::GetName.
 */
Scene::Scene(const std::string &name) :
	name(name), phase(Phase::INITIALIZED), phaseTs(0), startingPhaseTime(0)
{
}

/**
 * Switch to another phase.
 * @warning Scenes should not switch to the @c STOPPING phase themselves.
 *          Instead, they should call GameDirector::PopScene or
 *          GameDirector::ReplaceScene.
 * @param phase The new phase.
 * @return @c true if the phase change was successful, @c false otherwise.
 */
bool Scene::SetPhase(Phase::phase_t phase)
{
	if (this->phase != phase) {
		// Can't go back to the initial phase.
		if (phase == Phase::INITIALIZED) return false;

		// Once shutdown has started, it can't be stopped.
		if (this->phase == Phase::STOPPING ||
			this->phase == Phase::STOPPED)
		{
			return false;
		}

		// Save the starting phase duration in case the subclass wants to
		// use it later.
		if (phase == Phase::STARTING) {
			startingPhaseTime = GetPhaseDuration();
		}

		phaseTs = OS::Time();
		Phase::phase_t oldPhase = this->phase;
		this->phase = phase;

		OnPhaseChanged(oldPhase);

		return true;
	}
	else {
		return false;
	}
}

/**
 * Fired immediately after entering a new phase.
 * @param oldPhase The previous phase.
 * @see GetPhase()
 */
void Scene::OnPhaseChanged(Phase::phase_t oldPhase)
{
	// The default behavior is to act like the starting and stopping phases
	// don't even exist.
	switch (phase) {
		case Phase::STARTING:
			SetPhase(Phase::RUNNING);
			break;
		case Phase::STOPPING:
			SetPhase(Phase::STOPPED);
			break;
	}
}

}  // namespace HoverScript
}  // namespace Client
