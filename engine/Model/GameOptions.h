
// GameOptions.h
//
// Copyright (c) 2015 Michael Imamura.
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

#include <bitset>

#include "../Util/MR_Types.h"

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
namespace Model {

class MR_DllDeclare GameOptions
{
public:
	GameOptions() : weaponsEnabled(true) { }

public:
	char ToFlags() const;

public:
	//TODO: Handle this by blacklisting/allowing the list of weapon pickups.
	bool IsWeaponsEnabled() const { return weaponsEnabled; }
	void SetWeaponsEnabled(bool enabled) { weaponsEnabled = enabled; }

	/**
	 * Blacklists an object from spawning in the track.
	 * @param classId The object class ID (DLL ID is assumed to be ObjFac1).
	 */
	void Blacklist(MR_UInt16 classId)
	{
		if (classId < blacklist.size()) {
			blacklist[classId] = 1;
		}
	}

	/**
	 * Check if an object is allowed (by class ID).
	 * @param classId The object class ID (DLL ID is assumed to be ObjFac1).
	 * @return @c true if allowed, @c false if not.
	 */
	bool IsAllowed(MR_UInt16 classId) const
	{
		return
			classId >= blacklist.size() ||
			!blacklist[classId];
	}

private:
	bool weaponsEnabled;
	std::bitset<1104> blacklist;  ///< Class IDs from ObjFac1.
};


}  // namespace Model
}  // namespace HoverRace

#undef MR_DllDeclare
