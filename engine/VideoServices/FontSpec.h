
// FontSpec.h
// Header for specifying a specific typeface.
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
namespace VideoServices {

/**
 * Describes a typeface.
 * @author Michael Imamura
 */
struct MR_DllDeclare FontSpec {

	FontSpec(const std::string &name="Arial", int size=20,
		bool bold=false, bool italic=false) :
		name(name), size(size), bold(bold), italic(italic) { }

	std::string name;
	int size;
	bool bold;
	bool italic;
};

MR_DllDeclare inline bool operator==(const FontSpec &a, const FontSpec &b)
{
	return
		a.size == b.size &&
		a.bold == b.bold &&
		a.italic == b.italic &&
		a.name == b.name;
}

}  // namespace VideoServices
}  // namespace HoverRace

#undef MR_DllDeclare
