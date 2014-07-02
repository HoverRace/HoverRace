
// TrackFormatExn.h
//
// Copyright (c) 2014 Michael Imamura.
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

#include "../Exception.h"

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
namespace Model {

/**
 * Exception thrown when a track could not be loaded.
 * @author Michael Imamura
 */
class MR_DllDeclare TrackFormatExn : public Exception
{
	typedef Exception SUPER;

public:
	TrackFormatExn() : SUPER() { }
	TrackFormatExn(const std::string &msg) : SUPER(msg) { }
	TrackFormatExn(const char *msg) : SUPER(msg) { }
	virtual ~TrackFormatExn() noexcept { }
};

}  // namespace Model
}  // namespace HoverRace

#undef MR_DllDeclare
