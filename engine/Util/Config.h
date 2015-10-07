
// Config.h
//
// Copyright (c) 2008-2010, 2012-2015 Michael Imamura.
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

#ifndef _WIN32
	// Use XDG base directories on non-Win32.
#	include <basedir.h>
#endif

#include "OS.h"

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

// Forward declarations.
namespace HoverRace {
	namespace Parcel {
		class Bundle;
		class ResBundle;
		class TrackBundle;
	}
	namespace Util {
		namespace yaml {
			class Emitter;
			class MapNode;
		}
	}
}

namespace HoverRace {
namespace Util {

class ConfigExn : public std::exception
{
	using SUPER = std::exception;

public:
	ConfigExn() : SUPER() { }
	ConfigExn(const char* const &msg) : SUPER(), msg(msg) { }
	ConfigExn(const std::string &msg) : SUPER(), msg(msg) { }
	virtual ~ConfigExn() noexcept { }

	const char* what() const noexcept override { return msg.c_str(); }

private:
	std::string msg;
};

/**
 * Global configuration manager, shared by all aspects of the system.
 */
class MR_DllDeclare Config
{
private:
	Config(int verMajor, int verMinor, int verPatch, int verBuild,
		bool prerelease, const OS::path_t &mediaPath,
		const OS::path_t &sysCfgPath,
		const OS::path_t &file=OS::path_t());
public:
	~Config();

	static Config *Init(int verMajor, int verMinor, int verPatch, int verBuild,
		bool prerelease, const OS::path_t &mediaPath,
		const OS::path_t &sysCfgPath,
		const OS::path_t &path=OS::path_t());
	static void Shutdown();

	bool IsUnlinked() const;
	void SetUnlinked(bool unlinked);

	bool IsPrerelease() const;
	const std::string &GetVersion() const;
	const std::string &GetFullVersion() const;
	const int &GetBuild() const;
	const std::string &GetUserAgentId() const;

private:
	static OS::path_t GetDefaultBasePath();
	void AppendPackageSubdir(OS::path_t &path) const;

public:
	OS::path_t GetBaseDataPath() const;
	OS::path_t GetBaseConfigPath() const;
	OS::path_t GetConfigFilename() const;

	OS::path_t GetDefaultMediaPath();
	const OS::path_t &GetMediaPath() const;
	OS::path_t GetMediaPath(const std::string &file) const;

	const OS::path_t &GetUserTrackPath() const;
	OS::path_t GetUserTrackPath(const std::string &name) const;

	Parcel::ResBundle &GetResBundle() const { return *resBundle; }
	std::shared_ptr<Parcel::ResBundle> ShareResBundle() const { return resBundle; }

	Parcel::TrackBundle &GetTrackBundle() const { return *trackBundle; }

	/**
	 * Retrieve the track bundle.
	 * @return The track bundle (never @c nullptr).
	 */
	std::shared_ptr<Parcel::TrackBundle> ShareTrackBundle() const { return trackBundle; }

	OS::path_t GetScriptHelpPath(const std::string &className) const;

	OS::path_t GenerateScreenshotPath(const std::string &ext) const;

	static std::string GetDefaultRoomListUrl();
	static std::string GetDefaultUpdateServerUrl();
	static OS::path_t GetDefaultChatLogPath();
	static OS::path_t GetDefaultScreenshotPath();

	const std::string &GetDefaultFontName() const;
	const std::string &GetDefaultMonospaceFontName() const;
	const std::string &GetDefaultSymbolFontName() const;

public:
	static Config *GetInstance() { return instance; }

	void ResetToDefaults();

private:
	void LoadSystem();
	void LoadSystem(const OS::path_t &path);
public:
	void Load();
	void Save() const;

private:
	void SaveVersion(yaml::Emitter*) const;

public:
	static const std::string TRACK_EXT;

private:
	static Config *instance;
	bool unlinked;  ///< if @c true, will prevent saving config.
	OS::path_t dataPath;  ///< Base path for data files.
	OS::path_t cfgPath;  ///< Base path for config files.
	OS::path_t mediaPath;
	OS::path_t sysCfgPath;
	OS::path_t userTrackPath;
	std::shared_ptr<Parcel::ResBundle> resBundle;
	std::shared_ptr<Parcel::TrackBundle> trackBundle;
	int verBuild;
	bool prerelease;
	std::string shortVersion;
	std::string fullVersion;
	std::string userAgentId;
	std::string defaultFontName;
	std::string defaultMonospaceFontName;
	std::string defaultSymbolFontName;
#	ifndef _WIN32
		xdgHandle *xdg;
#	endif

public:
	struct app_t {
		OS::path_t localePath;
		OS::path_t mediaPath;

		void Load(yaml::MapNode*);
	} app;

	struct video_t {
		double gamma;
		double contrast;
		double brightness;

		double textScale;

		int xPos;
		int yPos;
		int xRes;
		int yRes;
		bool fullscreen;
		int fullscreenMonitorIndex;
		int xResFullscreen;
		int yResFullscreen;
		int fullscreenRefreshRate;

		bool stackedSplitscreen;

		void ResetToDefaults();
		void Load(yaml::MapNode*);
		void Save(yaml::Emitter*) const;
	} video;

	struct audio_t {
		double sfxVolume;

		void ResetToDefaults();
		void Load(yaml::MapNode*);
		void Save(yaml::Emitter*) const;
	} audio;

	struct misc_t {
		OS::path_t screenshotPath;

		void ResetToDefaults();
		void Load(yaml::MapNode*);
		void Save(yaml::Emitter*) const;
	} misc;

	struct player_t {
		std::string nickName;

		void ResetToDefaults();
		void Load(yaml::MapNode*);
		void Save(yaml::Emitter*) const;
	} player;

	struct net_t {
		std::string mainServer;
		std::string updateServer;
		bool autoUpdates;
		bool messageReceivedSound;
		bool messageReceivedSoundOnlyBg;
		bool logChats;
		OS::path_t logChatsPath;
		int udpRecvPort;
		int tcpRecvPort;
		int tcpServPort;
		static const int DEFAULT_UDP_RECV_PORT = 9531;
		static const int DEFAULT_TCP_RECV_PORT = 9531;
		static const int DEFAULT_TCP_SERV_PORT = 9530;
		//TODO: Proxy server settings.

		void ResetToDefaults();
		void Load(yaml::MapNode*);
		void Save(yaml::Emitter*) const;
	} net;

	static const int MAX_PLAYERS = 4;

	/// See Client/Control/Controller.h for the hash function
	struct controlsHash_t {
		int motorOn;
		int right;
		int left;
		int jump;
		int fire;
		int brake;
		int weapon;
		int lookBack;

		void Load(yaml::MapNode*);
		void Save(yaml::Emitter*) const;
	} controlsHash[MAX_PLAYERS];

	struct cameraHash_t {
		int zoomIn;
		int zoomOut;
		int panUp;
		int panDown;
		int reset;

		void Load(yaml::MapNode*);
		void Save(yaml::Emitter*) const;
	} cameraHash;

	struct ui_t {
		int consoleToggle;
		int consoleUp;
		int consoleDown;
		int consoleTop;
		int consoleBottom;
		int consolePrevCmd;
		int consoleNextCmd;
		int consoleHelp;
		int menuOk;
		int menuCancel;
		int menuExtra;
		int menuUp;
		int menuDown;
		int menuLeft;
		int menuRight;
		int menuNext;
		int menuPrev;

		void Load(yaml::MapNode*);
		void Save(yaml::Emitter*) const;
	} ui;

	struct runtime_t {
		bool silent;
		bool verboseLog;
		bool showFramerate;
		bool enableConsole;
		bool enableDebugOverlay;
		bool enableHud;
		bool noAccel;  ///< Disable accelerated (OpenGL) rendering.
		bool skipStartupWarning;
		bool profiling;
		std::vector<OS::path_t> initScripts;
	} runtime;
};

}  // namespace Util
}  // namespace HoverRace

#undef MR_DllDeclare
