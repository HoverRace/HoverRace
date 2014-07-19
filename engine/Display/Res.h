
// Res.h
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

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
namespace Display {

class ResLoadExn;

/**
 * Base class for loadable resources.
 * @author Michael Imamura
 */
template<class T>
class Res
{
	public:
		virtual ~Res() { }

	public:
		/**
		 * Open the stream for reading.
		 * @return The input stream.
		 * @throw ResourceLoadExn
		 */
		virtual std::istream Open() const;
};

/**
 * A resource failed to load.
 * @author Michael Imamura
 */
class ResLoadExn : Exception
{
	typedef Exception SUPER;

	public:
		ResLoadExn() : SUPER() { }
		ResLoadExn(const std::string &msg) : SUPER(msg) { }
		ResLoadExn(const char *msg) : SUPER(msg) { }
		virtual ~ResLoadExn() noexcept { }
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
