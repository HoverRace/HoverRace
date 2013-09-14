
// Rulebook.cpp
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

#include "HoverScript/SessionPeer.h"
#include "ClientSession.h"

#include "Rulebook.h"

namespace HoverRace {
namespace Client {

Rulebook::Rulebook(Script::Core *scripting, const std::string &name,
                   const std::string &description) :
	scripting(scripting), name(name), description(description),
	onPreGame(scripting), onPostGame(scripting)
{
}

void Rulebook::SetOnPreGame(const luabind::object &fn)
{
	onPreGame.AddHandler(fn);
}

void Rulebook::OnPreGame(HoverScript::SessionPeerPtr session) const
{
	onPreGame.CallHandlers(luabind::object(scripting->GetState(), session));
}

void Rulebook::SetOnPostGame(const luabind::object &fn)
{
	onPostGame.AddHandler(fn);
}

void Rulebook::OnPostGame(HoverScript::SessionPeerPtr session) const
{
	onPostGame.CallHandlers(luabind::object(scripting->GetState(), session));
}

}  // namespace Client
}  // namespace HoverRace

