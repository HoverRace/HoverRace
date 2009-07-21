
// NetExn.h
// The base network exception.
//
// Copyright (c) 2009 Michael Imamura.
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
namespace Net {

/**
 * A network exception.
 * @author Michael Imamura
 */
class MR_DllDeclare NetExn : public std::exception
{
	typedef std::exception SUPER;

	public:
		NetExn() : SUPER() { }
		NetExn(const char* const &msg) : SUPER(), msg(msg) { }
		NetExn(const std::string &msg) : SUPER(), msg(msg) { }
		virtual ~NetExn() { }

		virtual const char* what() const { return msg.c_str(); }

	private:
		std::string msg;
};

}  // namespace Net
}  // namespace HoverRace

#undef MR_DllDeclare
