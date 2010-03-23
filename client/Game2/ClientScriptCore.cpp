
// ClientScriptCore.cpp
// Scripting core configured for the client.
//
// Copyright (c) 2010 Michael Imamura.
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

#include "luabind/class_info.hpp"

#include "ConfigPeer.h"
#include "GamePeer.h"
#include "SessionPeer.h"

#include "ClientScriptCore.h"

namespace HoverRace {
namespace Client {

Script::Core *ClientScriptCore::Reset()
{
	SUPER::Reset();

	if (!classesRegistered) {

		lua_State *L = GetState();

		luabind::open(L);
#		ifdef _DEBUG
			// Enable class inspection.
			luabind::bind_class_info(L);
#		endif

		ConfigPeer::Register(this);
		GamePeer::Register(this);
		SessionPeer::Register(this);

		classesRegistered = true;
	}

	return this;
}

}  // namespace Client
}  // namespace HoverRace
