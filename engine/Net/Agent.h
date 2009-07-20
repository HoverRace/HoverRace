
// Agent.h
// Header for the network transfer agent (wrapper for libcurl).
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

#include <string>

#include "Transfer.h"

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
 * The network transfer agent (wrapper for libcurl).
 * @author Michael Imamura
 */
class MR_DllDeclare Agent
{
	public:
		Agent(const std::string &url="");
		virtual ~Agent();

	public:
		void SetUrl(const std::string &url);
		const std::string &GetUrl() const { return url; };

	public:
		void Get(std::string &buf);
		void Get(std::ostream &buf);

	private:
		std::string url;
};

}  // namespace Net
}  // namespace HoverRace

#undef MR_DllDeclare
