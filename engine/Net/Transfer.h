
// Transfer.h
// Header for the network transfer session.
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
#	define MR_PureVirtual __declspec(novtable)
#else
#	define MR_DllDeclare
#	define MR_PureVirtual
#endif

namespace HoverRace {
namespace Net {

/**
 * Interface for network transfers.
 * @author Michael Imamura
 */
class MR_DllDeclare MR_PureVirtual Transfer
{
	public:
		virtual bool IsComplete() const = 0;
};

}  // namespace Net
}  // namespace HoverRace

#undef MR_DllDeclare
