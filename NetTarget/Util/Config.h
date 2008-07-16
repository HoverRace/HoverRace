
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
class MR_Config
{
	private:
		static MR_Config *instance;
		std::string path;
		std::string version;

	protected:
		MR_DllDeclare MR_Config(const std::string &version, const std::string &file="");
	public:
		MR_DllDeclare virtual ~MR_Config();
	
		MR_DllDeclare static MR_Config *Init(const std::string &version, const std::string &path="");
		MR_DllDeclare static void Shutdown();

		MR_DllDeclare static std::string GetDefaultPath();
		MR_DllDeclare std::string GetConfigFilename() const;

		MR_DllDeclare std::string GetMediaPath() const;
		MR_DllDeclare std::string GetMediaPath(const std::string &file) const;

	public:
		MR_DllDeclare static MR_Config *GetInstance() { return instance; }

		MR_DllDeclare void ResetToDefaults();

		MR_DllDeclare void Load();
		MR_DllDeclare void Save();

	private:
		MR_DllDeclare void SaveVersion(yaml::Emitter*);

	public:
		struct cfg_video_t {
			double gamma;
			double contrast;
			double brightness;

			bool nativeBppFullscreen;

			MR_DllDeclare void Load(yaml::MapNode*);
			MR_DllDeclare void Save(yaml::Emitter*);
		} video;

		struct cfg_audio_t {
			float sfxVolume;

			MR_DllDeclare void Load(yaml::MapNode*);
			MR_DllDeclare void Save(yaml::Emitter*);
		} audio;

		struct cfg_misc_t {
			bool displayFirstScreen;
			bool introMovie;

			MR_DllDeclare void Load(yaml::MapNode*);
			MR_DllDeclare void Save(yaml::Emitter*);
		} misc;

		struct cfg_player_t {
			std::string nickName;

			MR_DllDeclare void Load(yaml::MapNode*);
			MR_DllDeclare void Save(yaml::Emitter*);
		} player;

		struct cfg_net_t {
			std::string mainServer;
			//TODO: Proxy server settings.

			MR_DllDeclare void Load(yaml::MapNode*);
			MR_DllDeclare void Save(yaml::Emitter*);
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

			MR_DllDeclare void Load(yaml::MapNode*, int);
			MR_DllDeclare void Save(yaml::Emitter*);
		} controls[MAX_PLAYERS];
};
