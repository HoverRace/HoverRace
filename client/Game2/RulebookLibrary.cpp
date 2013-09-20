
// RulebookLibrary.cpp
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

#include "../../engine/Script/Core.h"

#include "Rulebook.h"

#include "RulebookLibrary.h"

namespace HoverRace {
namespace Client {

RulebookLibrary::RulebookLibrary(Script::Core *scripting) :
	scripting(scripting),
	library(), sorted()
{
}

void RulebookLibrary::Add(RulebookPtr &rulebook)
{
	library.insert(library_t::value_type(rulebook->GetName(), rulebook));
	sorted.insert(rulebook);

	if (rulebook->GetName() == "Race") {
		defaultRulebook = rulebook;
	}
}

/**
 * Retrieve the default rulebook.
 * This is normally the "Race" rulebook, but if it's not defined, then
 * the first rulebook in sorted order is returned instead.
 * If no rulebooks are loaded, then a dummy rulebook will be returned.
 * @return The default rulebook (never null).
 */
std::shared_ptr<const Rulebook> RulebookLibrary::GetDefault() const
{
	if (defaultRulebook) {
		return defaultRulebook;
	}
	else {
		return library.empty() ?
			std::make_shared<Rulebook>(scripting, "Dummy", "Dummy", "", 1) :
			*sorted.cbegin();
	}
}

/**
 * Search for the Rulebook with the given name.
 * @param name The name to search for (case-sensitive).
 * @return The requested Rulebook, or a null RulebookPtr if
 *         there is no Rulebook with that name.
 */
std::shared_ptr<const Rulebook> RulebookLibrary::Find(const std::string &name)
{
	auto iter = library.find(name);
	return iter == library.end() ? RulebookPtr() : iter->second;
}

}  // namespace Client
}  // namespace HoverRace
