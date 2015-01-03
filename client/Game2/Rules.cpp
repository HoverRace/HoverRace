
// Rules.cpp
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

#include "Rulebook.h"

#include "Rules.h"

namespace HoverRace {
namespace Client {

Rules::Rules(std::shared_ptr<const Rulebook> rulebook) :
	rulebook(rulebook), rules(),
	gameOpts(0x7f)
{
	if (rulebook) {
		rules = rulebook->CreateDefaultRules();
		rulebook->Load();
	}
}

/**
 * Set the rulebook and reset the rules to their defaults.
 * @param rulebook The rulebook (may be @c nullptr).
 */
void Rules::SetRulebook(std::shared_ptr<const Rulebook> rulebook)
{
	rules = rulebook ? rulebook->CreateDefaultRules() : luabind::object();
	this->rulebook = std::move(rulebook);
}

}  // namespace Client
}  // namespace HoverRace
