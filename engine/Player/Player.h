
// Player.h
//
// Copyright (c) 2014, 2016 Michael Imamura.
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

#include "Profile.h"

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
	namespace MainCharacter {
		class MainCharacter;
	}
}

namespace HoverRace {
namespace Player {

/**
 * Base class for connected players.
 * @author Michael Imamura
 */
class MR_DllDeclare Player
{
public:
	enum class ConnectionState
	{
		DISCONNECTED,
		CONNECTING,
		CONNECTED,
		DISCONNECTING,
	};

public:
	Player() = delete;
	Player(std::shared_ptr<Profile> profile,
		bool local, bool human, bool competing);
	virtual ~Player() { }

public:
	virtual std::ostream &StreamOut(std::ostream &os) const;

public:
	/**
	 * Retrieve the display name of the player.
	 *
	 * This is not necessarily the same name as the profile, although it
	 * is based on the profile name.  In the case of multiple players
	 * with the same name, a suffix may be added to disambiguate them.
	 *
	 * @return The display name.
	 * @see SetNameSuffix(const std::string&)
	 */
	const std::string &GetName() const { return name; }

	void SetNameSuffix(const std::string &suffix);

	Profile *GetProfile() const { return profile.get(); }

	std::shared_ptr<Profile> ShareProfile() const { return profile; }

	bool IsLocal() const { return local; }

	bool IsHuman() const { return human; }

	bool IsCompeting() const { return competing; }

	ConnectionState GetConnectionState() const { return connectionState; }

	/**
	 * Retrieve the main character, if attached.
	 * @return The main character (may be @c nullptr if detached).
	 */
	MainCharacter::MainCharacter *GetMainCharacter() const { return mainCharacter; }

	/**
	 * Attach the main character (when joining a game session).
	 * @param mainCharacter The main character (may be @c nullptr to detach).
	 */
	void AttachMainCharacter(MainCharacter::MainCharacter *mainCharacter)
	{
		this->mainCharacter = mainCharacter;
	}

	/**
	 * Detach the main character (when leaving a game session).
	 */
	void DetachMainCharacter()
	{
		this->mainCharacter = nullptr;
	}

public:
	using nameChangedSignal_t = boost::signals2::signal<void()>;
	nameChangedSignal_t &GetNameChangedSignal() { return nameChangedSignal; }

	using connectionStateChanged_t = boost::signals2::signal<void()>;
	connectionStateChanged_t &GetConnectionStateChangedSignal() { return connectionStateChangedSignal; }

protected:
	void SetConnectionState(ConnectionState state);
	virtual void Disconnect() = 0;

private:
	std::string name;
	std::shared_ptr<Profile> profile;
	bool local;
	bool human;
	bool competing;
	ConnectionState connectionState;
	MainCharacter::MainCharacter *mainCharacter;
	nameChangedSignal_t nameChangedSignal;
	connectionStateChanged_t connectionStateChangedSignal;
};

inline std::ostream &operator<<(std::ostream &os, const Player &p)
{
	return p.StreamOut(os);
}

}  // namespace Player
}  // namespace HoverRace

#undef MR_DllDeclare
