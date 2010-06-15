
// Config.h
// Header for the configuration system.
//
// Copyright (c) 2008, 2009, 2010 Michael Imamura.
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

#include "OS.h"

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

// Forward declarations.
namespace yaml {
	class Emitter;
	class MapNode;
}
namespace HoverRace {
	namespace Parcel {
		class Bundle;
		typedef boost::shared_ptr<Bundle> BundlePtr;
		class TrackBundle;
		typedef boost::shared_ptr<TrackBundle> TrackBundlePtr;
	}
}

namespace HoverRace {
namespace Util {

class ConfigExn : public std::exception
{
	typedef std::exception SUPER;

	public:
		ConfigExn() : SUPER() { }
		ConfigExn(const char* const &msg) : SUPER(), msg(msg) { }
		ConfigExn(const std::string &msg) : SUPER(), msg(msg) { }
		virtual ~ConfigExn() throw() { }

		virtual const char* what() const throw() { return msg.c_str(); }

	private:
		std::string msg;
};
	
/**
 * Global configuration manager, shared by all aspects of the system.
 */
class MR_DllDeclare Config
{
	private:
		static Config *instance;
		bool unlinked;  ///< if @c true, will prevent saving config.
		OS::path_t path;
		OS::path_t mediaPath;
		OS::path_t userTrackPath;
		Parcel::TrackBundlePtr trackBundle;
		int verMajor;
		int verMinor;
		int verPatch;
		int verBuild;
		bool prerelease;
		std::string shortVersion;
		std::string fullVersion;
		std::string userAgentId;
		std::string defaultFontName;

	private:
		Config(int verMajor, int verMinor, int verPatch, int verBuild,
			bool prerelease, const OS::path_t &file=OS::path_t());
	public:
		~Config();
	
		static Config *Init(int verMajor, int verMinor, int verPatch, int verBuild,
			bool prerelease, const OS::path_t &path=OS::path_t());
		static void Shutdown();

		bool IsUnlinked() const;
		void SetUnlinked(bool unlinked);

		bool IsPrerelease() const;
		const std::string &GetVersion() const;
		const std::string &GetFullVersion() const;
		const int &GetBuild() const;
		const std::string &GetUserAgentId() const;

		static OS::path_t GetDefaultPath();
		OS::path_t GetConfigFilename() const;

		const OS::path_t &GetMediaPath() const;
		OS::path_t GetMediaPath(const std::string &file) const;

		const OS::path_t &GetUserTrackPath() const;
		OS::path_t GetUserTrackPath(const std::string &name) const;
		Parcel::TrackBundlePtr GetTrackBundle() const;

		OS::path_t GetScriptHelpPath(const std::string &className) const;

		static std::string GetDefaultRoomListUrl();
		static std::string GetDefaultUpdateServerUrl();
		static OS::path_t GetDefaultChatLogPath();

		const std::string &GetDefaultFontName() const;

	public:
		static Config *GetInstance() { return instance; }

		void ResetToDefaults();

		void Load();
		void Save();

	private:
		void SaveVersion(yaml::Emitter*);

	public:
		static const std::string TRACK_EXT;

		struct cfg_video_t {
			double gamma;
			double contrast;
			double brightness;

			bool nativeBppFullscreen;

			int xPos;
			int yPos;
			int xRes;
			int yRes;
			std::string fullscreenMonitor;
			int xResFullscreen; ///< for fullscreen
			int yResFullscreen; ///< for fullscreen

			void Load(yaml::MapNode*);
			void Save(yaml::Emitter*);
		} video;

		struct cfg_audio_t {
			float sfxVolume;

			void Load(yaml::MapNode*);
			void Save(yaml::Emitter*);
		} audio;

		struct cfg_misc_t {
			bool displayFirstScreen;
			bool introMovie;
			bool aloneWarning; /// warn a player if he launches a game alone

			void Load(yaml::MapNode*);
			void Save(yaml::Emitter*);
		} misc;

		struct cfg_player_t {
			std::string nickName;

			void Load(yaml::MapNode*);
			void Save(yaml::Emitter*);
		} player;

		struct cfg_net_t {
			std::string mainServer;
			std::string updateServer;
			bool autoUpdates;
			bool logChats;
			OS::path_t logChatsPath;
			int udpRecvPort;
			int tcpRecvPort;
			int tcpServPort;
			static const int DEFAULT_UDP_RECV_PORT = 9531;
			static const int DEFAULT_TCP_RECV_PORT = 9531;
			static const int DEFAULT_TCP_SERV_PORT = 9530;
			//TODO: Proxy server settings.

			void Load(yaml::MapNode*);
			void Save(yaml::Emitter*);
		} net;

		struct cfg_control_t {
			int inputType; /// OISKeyboard, OISMouse, OISJoyStick

			// keyboard
			int kbdBinding; // see OIS KeyCode class

			// mouse or joystick
			int button;
			int axis;
			int direction; // for axis as well as POV
			int pov; // joystick only
			int slider; // joystick only
			int sensitivity; // defines how quickly we get to the maximum value
			int joystickId;

			MR_DllDeclare static cfg_control_t Key(int kc);
			MR_DllDeclare void SetKey(int kc);
			MR_DllDeclare bool IsKey(int kc) const;

			void Load(yaml::MapNode *);
			void Save(yaml::Emitter *);
		};

		static const int MAX_PLAYERS = 4;
		struct cfg_controls_t {
			cfg_control_t motorOn;
			cfg_control_t right;
			cfg_control_t left;
			cfg_control_t jump;
			cfg_control_t fire;
			cfg_control_t brake;
			cfg_control_t weapon;
			cfg_control_t lookBack;

			void Load(yaml::MapNode*);
			void Save(yaml::Emitter*);
		} controls[MAX_PLAYERS];

		struct cfg_ui_t {
			// Not configurable yet.
			cfg_control_t console;
		} ui;

		struct cfg_runtime_t {
			bool silent;
			bool aieeee;
			bool showFramerate;
			bool enableConsole;
			OS::path_t initScript;
		} runtime;
};

}  // namespace Util
}  // namespace HoverRace

#undef MR_DllDeclare
