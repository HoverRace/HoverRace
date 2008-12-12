
/* Config.h
     Header for MR_Config class. */

#pragma once

#include <string>

#ifdef MR_UTIL
#define MR_DllDeclare   __declspec( dllexport )
#else
#define MR_DllDeclare   __declspec( dllimport )
#endif

// Forward declarations.
namespace yaml {
	class Emitter;
	class MapNode;
}

/**
 * Global configuration manager, shared by all aspects of the system.
 */
class MR_DllDeclare MR_Config
{
	private:
		static MR_Config *instance;
		std::string path;
		int verMajor;
		int verMinor;
		int verPatch;
		int verBuild;
		std::string shortVersion;
		std::string fullVersion;

	private:
		MR_Config(int verMajor, int verMinor, int verPatch, int verBuild, const std::string &file="");
	public:
		~MR_Config();
	
		static MR_Config *Init(int verMajor, int verMinor, int verPatch, int verBuild, const std::string &path="");
		static void Shutdown();

		std::string GetVersion() const;
		std::string GetFullVersion() const;

		static std::string GetDefaultPath();
		std::string GetConfigFilename() const;

		std::string GetMediaPath() const;
		std::string GetMediaPath(const std::string &file) const;

		std::string GetTrackPath() const;
		std::string GetTrackPath(const std::string &file) const;

	public:
		static MR_Config *GetInstance() { return instance; }

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
};
