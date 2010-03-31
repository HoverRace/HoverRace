
// Peer.h
// Base class for script peers that extend Object.
//
// Copyright (c) 2009, 2010 Michael Imamura.
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

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
	namespace Script {
		class Core;
	}
}

namespace HoverRace {
namespace Script {

/**
 * Base class for script peers that extend Object
 * @author Michael Imamura
 */
class MR_DllDeclare Peer {
	public:
		Peer(Core *scripting, const std::string &name);
		virtual ~Peer();

		static void Register(Core *scripting);

		Core *GetScripting() const { return scripting; }

	public:
		void LHelp();
		void LHelp(const std::string &methodName);

	private:
		Core *scripting;
		std::string name;
};

}  // namespace Script
}  // namespace HoverRace

#undef MR_DllDeclare
