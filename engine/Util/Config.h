
// Config.h
// Header for the configuration system.
//
// Copyright (c) 2008, 2009 Michael Imamura.
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

// Forward declarations.
namespace yaml {
	class Emitter;
	class MapNode;
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
		std::string path;
		int verMajor;
		int verMinor;
		int verPatch;
		int verBuild;
		bool prerelease;
		std::string shortVersion;
		std::string fullVersion;
		std::string userAgentId;

	private:
		Config(int verMajor, int verMinor, int verPatch, int verBuild,
			bool prerelease, const std::string &file="");
	public:
		~Config();
	
		static Config *Init(int verMajor, int verMinor, int verPatch, int verBuild,
			bool prerelease, const std::string &path="");
		static void Shutdown();

		bool IsPrerelease() const;
		const std::string &GetVersion() const;
		const std::string &GetFullVersion() const;
		const std::string &GetUserAgentId() const;

		static std::string GetDefaultPath();
		std::string GetConfigFilename() const;

		std::string GetMediaPath() const;
		std::string GetMediaPath(const std::string &file) const;

		std::string GetTrackPath() const;
		std::string GetTrackPath(const std::string &file) const;

	public:
		static Config *GetInstance() { return instance; }

		void ResetToDefaults();

		void Load();
		void Save();

	private:
		void SaveVersion(yaml::Emitter*);

	public:
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
			int xResFullscreen; /// for fullscreen
			int yResFullscreen; /// for fullscreen

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
			int udpRecvPort;
			int tcpRecvPort;
			int tcpServPort;
			//TODO: Proxy server settings.

			void Load(yaml::MapNode*);
			void Save(yaml::Emitter*);
		} net;

		static const int MAX_PLAYERS = 4;
		struct cfg_controls_t {
			int motorOn;
			int right;
			int left;
			int jump;
			int fire;
			int brake;
			int weapon;
			int lookBack;

			void Load(yaml::MapNode*, int);
			void Save(yaml::Emitter*);
		} controls[MAX_PLAYERS];

		struct cfg_runtime_t {
			bool silent;
			bool ohMyGodItsAllBroken;
		} runtime;
};

}  // namespace Util
}  // namespace HoverRace
