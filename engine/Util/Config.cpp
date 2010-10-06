
// Config.cpp
// Configuration system.
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

#include "StdAfx.h"

#include <limits.h>
#include <stdlib.h>

#include <string>
#include <sstream>
#include <exception>

#ifdef _WIN32
#	include <windows.h>
#	include <shlobj.h>
#	include <lmcons.h>
#else
#	include <sys/types.h>
#	include <pwd.h>
#	include <unistd.h>
#endif

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include <OIS/OISPrereqs.h>
#include <OIS/OISKeyboard.h>

#include "yaml/Emitter.h"
#include "yaml/MapNode.h"
#include "yaml/ScalarNode.h"
#include "yaml/SeqNode.h"
#include "yaml/Parser.h"

#include "../Parcel/Bundle.h"
#include "../Parcel/TrackBundle.h"
#include "Str.h"

#include "Config.h"

#ifndef _WIN32
#	include "../../config.h"
#endif

namespace fs = boost::filesystem;

#ifdef _WIN32
	#define DIRSEP "\\"
#else
	#define DIRSEP "/"
#endif
#define CONFIG_FILENAME			"config.yml"
#define PREREL_CONFIG_FILENAME	"config-testing.yml"

#define DEFAULT_NICKNAME		"Player"
#define DEFAULT_MAIN_SERVER		"www.hoverrace.com/imr/rl.php"
#define DEFAULT_UPDATE_SERVER	"www.hoverrace.com/updates/updates.php"
#define OLD_MAIN_SERVER			"66.197.183.245/~sirbrock/imr/rl.php"

#define READ_BOOL(root,name) \
	{\
		yaml::ScalarNode *_scalar = dynamic_cast<yaml::ScalarNode*>((root)->Get(#name)); \
		if (_scalar != NULL) (name) = _scalar->AsBool(name); \
	}

#define READ_INT(root,name,min,max) \
	{\
		yaml::ScalarNode *_scalar = dynamic_cast<yaml::ScalarNode*>((root)->Get(#name)); \
		if (_scalar != NULL) (name) = _scalar->AsInt(name, min, max); \
	}

#define READ_FLOAT(root,name,min,max) \
	{\
		yaml::ScalarNode *_scalar = dynamic_cast<yaml::ScalarNode*>((root)->Get(#name)); \
		if (_scalar != NULL) (name) = _scalar->AsFloat(name, min, max); \
	}

#define READ_DOUBLE(root,name,min,max) \
	{\
		yaml::ScalarNode *_scalar = dynamic_cast<yaml::ScalarNode*>((root)->Get(#name)); \
		if (_scalar != NULL) (name) = _scalar->AsDouble(name, min, max); \
	}

#define READ_STRING(root,name) \
	{\
		yaml::ScalarNode *_scalar = dynamic_cast<yaml::ScalarNode*>((root)->Get(#name)); \
		if (_scalar != NULL) (name) = _scalar->AsString(); \
	}

#define READ_PATH(root,name) \
	{\
		yaml::ScalarNode *_scalar = dynamic_cast<yaml::ScalarNode*>((root)->Get(#name)); \
		if (_scalar != NULL) (name) = _scalar->AsPath(); \
	}

#define EMIT_VAR(emitter,name) \
	(emitter)->MapKey(#name); \
	(emitter)->Value(name);

namespace HoverRace {
namespace Util {

const std::string Config::TRACK_EXT(".trk");

Config *Config::instance = NULL;

/**
 * Create a new instance using the specified directory.
 * @param verMajor The major (first) component of the version number.
 * @param verMinor The minor (second) component of the version number.
 * @param verPatch The patch (third) component of the version number.
 * @param verBuild The build (fourth) component of the version number.
 * @param prerelease Whether this build is a development version.
 * @param path (Optional) The directory for storing configuration data.
 *             The default is to use {@link #GetDefaultPath()}.
 */
Config::Config(int verMajor, int verMinor, int verPatch, int verBuild,
					 bool prerelease, const OS::path_t &path) :
	unlinked(false),
	verMajor(verMajor), verMinor(verMinor), verPatch(verPatch), verBuild(verBuild),
	prerelease(prerelease)
{
	OS::path_t homePath = GetDefaultPath();
	this->path = path.empty() ? homePath : path;

	std::ostringstream oss;
	oss << verMajor << '.' << verMinor;
	if (verPatch == 0) {
		shortVersion = oss.str();
		oss << '.' << verPatch;
	} else {
		oss << '.' << verPatch;
		shortVersion = oss.str();
	}
	oss << '.' << verBuild;
	fullVersion = oss.str();

	userAgentId = PACKAGE_NAME "/";
	userAgentId += shortVersion;
	userAgentId += " (" PLATFORM_NAME ")";

#	ifdef _WIN32
		defaultFontName = "Arial";
#	else
		defaultFontName = "sans";
#	endif
	//TODO: Check if we successfully loaded our private fonts.

	// Set initial defaults.
	ResetToDefaults();

#	ifdef _WIN32
		mediaPath = L"..";
		mediaPath /= L"share";
#	else
		//TODO: Get directory from configure.
		mediaPath = "..";
		mediaPath /= "share";
		mediaPath /= PACKAGE;
#	endif

	Str::UP tracksDirName("Tracks");

	userTrackPath = homePath / tracksDirName;

	Parcel::BundlePtr mediaTrackBundle(new Parcel::Bundle(mediaPath / tracksDirName));
#	ifdef _WIN32
		trackBundle = boost::make_shared<Parcel::TrackBundle>(userTrackPath,
			boost::make_shared<Parcel::Bundle>((OS::cpstr_t)Str::UP("track_source"),  // Historical.
			boost::make_shared<Parcel::Bundle>((OS::cpstr_t)tracksDirName,  // Historical.
			mediaTrackBundle)));
#	else
		trackBundle = boost::make_shared<Parcel::TrackBundle>(userTrackPath,
			mediaTrackBundle);
#	endif

}

Config::~Config()
{
}


/**
 * Initialize the singleton instance.
 * @param verMajor The major (first) component of the version number.
 * @param verMinor The minor (second) component of the version number.
 * @param verPatch The patch (third) component of the version number.
 * @param verBuild The build (fourth) component of the version number.
 * @param prerelease Whether this build is a development version.
 * @param path (Optional) The directory for storing configuration data.
 *             The default is to use {@link #GetDefaultPath()}.
 * @return The config instance.
 */
Config *Config::Init(int verMajor, int verMinor, int verPatch, int verBuild,
						   bool prerelease, const OS::path_t &path)
{
	if (instance == NULL) {
		instance = new Config(verMajor, verMinor, verPatch, verBuild, prerelease, path);
	}
	return instance;
}

/**
 * Destroy the singleton instance.
 */
void Config::Shutdown()
{
	if (instance != NULL) {
		delete instance;
		instance = NULL;
	}
}

/**
 * Check if saving the config to disk is disabled.
 * @return @c true if saving is disabled.
 * @see #SetUnlinked(bool)
 */
bool Config::IsUnlinked() const
{
	return unlinked;
}

/**
 * Enable or disable saving the configuration to disk.
 * This is useful for testing, since it allows temporary changes
 * without overwriting the config previously saved to disk.
 * @param unlinked @c true to prevent saving config to disk.
 */
void Config::SetUnlinked(bool unlinked)
{
	this->unlinked = unlinked;
}

/**
 * Check if this build is a prerelease (development) version.
 * @return @c true if prerelease.
 */
bool Config::IsPrerelease() const
{
	return prerelease;
}

/**
 * Retrieve the short form (x.x or x.x.x) of the version number.
 * @return The version number (always at least two components).
 */
const std::string &Config::GetVersion() const
{
	return shortVersion;
}

/**
 * Retrieve the long form (x.x.x.x) of the version number.
 * @return The version number (the same that was passed to the constructor).
 */
const std::string &Config::GetFullVersion() const
{
	return fullVersion;
}

/**
 * Retrieve the build number of the version.
 * @return The build number (that was passed to the constructor).
 */
const int &Config::GetBuild() const
{
	return verBuild;
}

/**
 * Retrieve the user agent identifier used for HTTP requests.
 * @return The user agent string.
 */
const std::string &Config::GetUserAgentId() const
{
	return userAgentId;
}

/**
 * Retrieve the OS-specific default configuration directory.
 * @return The fully-qualified path.
 * @throws ConfigExn if unable to retrieve the default directory.
 */
OS::path_t Config::GetDefaultPath()
{
#ifdef _WIN32
	wchar_t dpath[MAX_PATH] = {0};
	HRESULT hr = 
		SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, dpath);
	if (SUCCEEDED(hr)) {
		OS::path_t retv(dpath);
		retv /= L"HoverRace.com";
		retv /= L"HoverRace";
		return retv;
	}
	else {
		throw ConfigExn("Unable to determine configuration directory");
	}
#else
	char *home = getenv("HOME");
	if (home == NULL) {
		passwd *pw = getpwuid(geteuid());
		home = pw->pw_dir;
	}
	if (home != NULL) {
		OS::path_t retv(home);
		retv /= ".hoverrace";
		return retv;
	}
	else {
		throw ConfigExn(
			"Unable to determine configuration directory "
			"(HOME environment variable not set)");
	}
#endif
}

OS::path_t Config::GetConfigFilename() const
{
	OS::path_t retv(path);
	if (prerelease) {
		retv /= (OS::cpstr_t)Str::UP(PREREL_CONFIG_FILENAME);
	}
	else {
		retv /= (OS::cpstr_t)Str::UP(CONFIG_FILENAME);
	}
	return retv;
}

/**
 * Retrieve the directory for media files.
 * @return The directory path (may be relative).
 */
const OS::path_t &Config::GetMediaPath() const
{
	return mediaPath;
}

/**
 * Retrieve the path to a media file.
 * @param file The media filename (may not be blank).
 * @return The file path (may be relative).
 */
OS::path_t Config::GetMediaPath(const std::string &file) const
{
	return mediaPath / (OS::cpstr_t)Str::UP(file.c_str());
}

/**
 * Retrieve the path to where downloaded and user-installed tracks are stored.
 * @return The path (never empty, may be relative).
 */
const OS::path_t &Config::GetUserTrackPath() const
{
	return userTrackPath;
}

/**
 * Retrieve the path to a downloaded track.
 * @param name The track filename (may not be blank).
 *             If the filename does not end in ".trk", it will be appended
 *             automatically.
 * @return The directory path (may be relative).
 */
OS::path_t Config::GetUserTrackPath(const std::string &name) const
{
	std::string filename(name);
	if (!boost::ends_with(filename, TRACK_EXT)) {
		filename += TRACK_EXT;
	}
	return userTrackPath / (OS::cpstr_t)Str::UP(filename.c_str());
}

/**
 * Retrieve the track bundle.
 * @return The track bundle (never @c NULL).
 */
Parcel::TrackBundlePtr Config::GetTrackBundle() const
{
	return trackBundle;
}

/**
 * Retrieve the path to the help file for a class in the scripting API.
 * @param className The name of the class.
 * @return The directory path (may be relative).
 */
OS::path_t Config::GetScriptHelpPath(const std::string &className) const
{
	std::string filename(className);
	filename += ".yml";

	OS::path_t retv(GetMediaPath());
	retv /= (OS::cpstr_t)Str::UP("scripts");
	retv /= (OS::cpstr_t)Str::UP("help");
	retv /= (OS::cpstr_t)Str::UP(filename.c_str());
	return retv;
}

/**
 * Retrieve the default URL for the IMR room list.
 * @return The URL (without the initial "http://").
 */
std::string Config::GetDefaultRoomListUrl()
{
	return DEFAULT_MAIN_SERVER;
}

/**
 * Retrieve the default URL for the update server list.
 * @return The URL (without the initial "http://").
 */
std::string Config::GetDefaultUpdateServerUrl()
{
	return DEFAULT_UPDATE_SERVER;
}

/**
 * Retrieve the default path for the chat log.
 * @return The fully-qualified path (may be empty if base path could not
 *         be retrieved from the system).
 */
OS::path_t Config::GetDefaultChatLogPath()
{
#ifdef _WIN32
	wchar_t dpath[MAX_PATH] = {0};
	HRESULT hr = 
		SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, dpath);
	if (SUCCEEDED(hr)) {
		OS::path_t retv(dpath);
		retv /= L"HoverRace Chat";
		return retv;
	}
	else {
		ASSERT(FALSE);
		return OS::path_t();
	}
#else
	char *home = getenv("HOME");
	if (home != NULL) {
		OS::path_t retv(home);
		retv /= "HoverRace Chat";
		return retv;
	}
	else {
		return OS::path_t();
	}
#endif
}

/**
 * Retrieve the default UI font name.
 * @return The font name (never empty).
 */
const std::string &Config::GetDefaultFontName() const
{
	return defaultFontName;
}

/**
 * Resets the configuration to the default "factory" settings.
 */
void Config::ResetToDefaults()
{
	video.gamma = 1.2;
	video.contrast = 0.95;
	video.brightness = 0.95;
	video.nativeBppFullscreen = false;
	video.xPos = 150;
	video.yPos = 150;
	video.xRes = 800;
	video.yRes = 600;

	// set current resolution to default
#ifdef _WIN32
	// use the primary monitor resolution (don't default to multiple monitors)
	video.xResFullscreen = GetSystemMetrics(SM_CXSCREEN);
	video.yResFullscreen = GetSystemMetrics(SM_CYSCREEN);
#else
	// I'm sure there's an X call that would do this, but that's for another day
	video.xResFullscreen = 800;
	video.yResFullscreen = 600;
#endif

	audio.sfxVolume = 1.0;

	misc.displayFirstScreen = true;
	misc.introMovie = true;
	misc.aloneWarning = true;

	// Get current user name as default nickname.
#ifdef _WIN32
	char buf[UNLEN + 1];
	DWORD bsize = sizeof(buf);
	if (GetUserName(buf, &bsize)) {
		buf[UNLEN] = '\0';
		player.nickName = buf;
	}
	else {
		player.nickName = DEFAULT_NICKNAME;
	}
#else
	char *buf = getlogin();
	if (buf != NULL) {
		player.nickName = buf;
	}
	else {
		player.nickName = DEFAULT_NICKNAME;
	}
#endif

	net.mainServer = DEFAULT_MAIN_SERVER;
	net.updateServer = DEFAULT_UPDATE_SERVER;
	net.autoUpdates = true;
	net.logChats = false;
	net.logChatsPath = GetDefaultChatLogPath();
	net.udpRecvPort = cfg_net_t::DEFAULT_UDP_RECV_PORT;
	net.tcpRecvPort = cfg_net_t::DEFAULT_TCP_RECV_PORT;
	net.tcpServPort = cfg_net_t::DEFAULT_TCP_SERV_PORT;

	// Default controls.
	// values pulled from OIS
	controls_hash[0].motorOn  = OIS::KC_LSHIFT   << 8;
	controls_hash[0].right    = OIS::KC_RIGHT    << 8;
	controls_hash[0].left     = OIS::KC_LEFT     << 8;
	controls_hash[0].jump     = OIS::KC_UP       << 8;
	controls_hash[0].fire     = OIS::KC_LCONTROL << 8;
	controls_hash[0].brake    = OIS::KC_DOWN     << 8;
	controls_hash[0].weapon   = OIS::KC_TAB      << 8;
	controls_hash[0].lookBack = OIS::KC_END      << 8;

	controls_hash[1].motorOn  = OIS::KC_F        << 8;
	controls_hash[1].right    = OIS::KC_D        << 8;
	controls_hash[1].left     = OIS::KC_A        << 8;
	controls_hash[1].jump     = OIS::KC_S        << 8;
	controls_hash[1].fire     = OIS::KC_R        << 8;
	controls_hash[1].brake    = OIS::KC_W        << 8;
	controls_hash[1].weapon   = OIS::KC_Q        << 8;
	controls_hash[1].lookBack = OIS::KC_I        << 8;

	// set these to disabled (0x0000<code>)
	controls_hash[2].motorOn  = 0;
	controls_hash[2].right    = 1;
	controls_hash[2].left     = 2;
	controls_hash[2].jump     = 3;
	controls_hash[2].fire     = 4;
	controls_hash[2].brake    = 5;
	controls_hash[2].weapon   = 6;
	controls_hash[2].lookBack = 7;

	controls_hash[3].motorOn  = 8;
	controls_hash[3].right    = 9;
	controls_hash[3].left     = 10;
	controls_hash[3].jump     = 11;
	controls_hash[3].fire     = 12;
	controls_hash[3].brake    = 13;
	controls_hash[3].weapon   = 14;
	controls_hash[3].lookBack = 15;

	ui.console.SetKey(OIS::KC_F12);
	ui.console_hash = OIS::KC_F12 << 8;

	runtime.silent = false;
	runtime.aieeee = false;
	runtime.showFramerate = false;
	runtime.enableConsole = true;
}

/**
 * Load the configuration.
 * Does nothing if the configuration file doesn't exist.
 * @throws ConfigExn If an error occurs while reading.
 */
void Config::Load()
{
	const OS::path_t cfgfile(GetConfigFilename());
	
	FILE *in = OS::FOpen(cfgfile, "rb");
	if (in == NULL) {
		// File doesn't exist.
		// That's perfectly fine; we'll use the defaults.
		return;
	}

	yaml::Parser *parser = NULL;
	try {
		parser = new yaml::Parser(in);
		yaml::Node *node = parser->GetRootNode();

		yaml::MapNode *root = dynamic_cast<yaml::MapNode*>(node);
		if (root != NULL) {
			// Read the config's reported version so we can pass it to the
			// section loaders, so they can translate if necessary.
			// If no version, then assume the latest version.
			yaml::ScalarNode *verNode = dynamic_cast<yaml::ScalarNode*>(root->Get("version"));
			std::string cfgVer((verNode == NULL) ? shortVersion : verNode->AsString());

			video.Load(dynamic_cast<yaml::MapNode*>(root->Get("video")));
			audio.Load(dynamic_cast<yaml::MapNode*>(root->Get("audio")));
			misc.Load(dynamic_cast<yaml::MapNode*>(root->Get("misc")));
			player.Load(dynamic_cast<yaml::MapNode*>(root->Get("player")));
			net.Load(dynamic_cast<yaml::MapNode*>(root->Get("net")));

			// Get the controls.
			yaml::SeqNode *ctlseqh = dynamic_cast<yaml::SeqNode*>(root->Get("controls_hash"));
			if (ctlseqh != NULL) {
				int i = 0;
				BOOST_FOREACH(yaml::Node *node, *ctlseqh) {
					controls_hash[i++].Load(dynamic_cast<yaml::MapNode*>(node));
				}
			}
		}

		delete parser;
	}
	catch (yaml::EmptyDocParserExn&) {
		// Ignore.
	}
	catch (yaml::ParserExn &ex) {
		if (parser != NULL) delete parser;
		fclose(in);
		throw ConfigExn(ex.what());
	}

	fclose(in);
}

/**
 * Save the configuration.
 * @throws std::exception If an error occurs while saving.
 */
void Config::Save()
{
	if (unlinked) return;

	const OS::path_t cfgfile(GetConfigFilename());

	// Create the config directory.
	if (!fs::exists(path)) {
		if (!fs::create_directories(path)) {
			std::string msg("Unable to create directory: ");
			msg += (const char*)Str::PU(path.file_string().c_str());
			throw ConfigExn(msg.c_str());
		}
	}

	FILE *out = OS::FOpen(cfgfile, "wb");
	if (out == NULL) {
		std::string msg("Unable to create configuration file: ");
		msg += (const char*)Str::PU(cfgfile.file_string().c_str());
		throw ConfigExn(msg.c_str());
	}

	yaml::Emitter *emitter = NULL;
	try {
		emitter = new yaml::Emitter(out);
		emitter->StartMap();

		SaveVersion(emitter);
		video.Save(emitter);
		audio.Save(emitter);
		misc.Save(emitter);
		player.Save(emitter);
		net.Save(emitter);
		
		// Save list of controls.
		emitter->MapKey("controls_hash");
		emitter->StartSeq();
		for (int i = 0; i < MAX_PLAYERS; ++i) {
			controls_hash[i].Save(emitter);
		}
		emitter->EndSeq();

		emitter->EndMap();
		delete emitter;
	}
	catch (yaml::EmitterExn &ex) {
		if (emitter != NULL) delete emitter;
		fclose(out);
		throw std::exception(ex);
	}

	fclose(out);
}

void Config::SaveVersion(yaml::Emitter *emitter)
{
	std::string &version = shortVersion;
	EMIT_VAR(emitter, version);
}

// video ///////////////////////////////////////////////////////////////////////

void Config::cfg_video_t::Load(yaml::MapNode *root)
{
	if (root == NULL) return;

	READ_DOUBLE(root, gamma, 0.0, 2.0);
	READ_DOUBLE(root, contrast, 0.0, 1.0);
	READ_DOUBLE(root, brightness, 0.0, 1.0);

	READ_BOOL(root, nativeBppFullscreen);

	READ_INT(root, xPos, -32768, 32768);
	READ_INT(root, yPos, -32768, 32768);
	READ_INT(root, xRes, 0, 32768);
	READ_INT(root, yRes, 0, 32768);
	READ_STRING(root, fullscreenMonitor);
	READ_INT(root, xResFullscreen, 0, 32768);
	READ_INT(root, yResFullscreen, 0, 32768);
}

void Config::cfg_video_t::Save(yaml::Emitter *emitter)
{
	emitter->MapKey("video");
	emitter->StartMap();

	EMIT_VAR(emitter, gamma);
	EMIT_VAR(emitter, contrast);
	EMIT_VAR(emitter, brightness);

	EMIT_VAR(emitter, nativeBppFullscreen);

	EMIT_VAR(emitter, xPos);
	EMIT_VAR(emitter, yPos);
	EMIT_VAR(emitter, xRes);
	EMIT_VAR(emitter, yRes);
	EMIT_VAR(emitter, fullscreenMonitor);
	EMIT_VAR(emitter, xResFullscreen);
	EMIT_VAR(emitter, yResFullscreen);

	emitter->EndMap();
}

// audio ///////////////////////////////////////////////////////////////////////

void Config::cfg_audio_t::Load(yaml::MapNode *root)
{
	if (root == NULL) return;

	READ_FLOAT(root, sfxVolume, 0.0f, 1.0f);
}

void Config::cfg_audio_t::Save(yaml::Emitter *emitter)
{
	emitter->MapKey("audio");
	emitter->StartMap();

	EMIT_VAR(emitter, sfxVolume);

	emitter->EndMap();
}

// misc ////////////////////////////////////////////////////////////////////////

void Config::cfg_misc_t::Load(yaml::MapNode *root)
{
	if (root == NULL) return;

	READ_BOOL(root, displayFirstScreen);
	READ_BOOL(root, introMovie);
	READ_BOOL(root, aloneWarning);
}

void Config::cfg_misc_t::Save(yaml::Emitter *emitter)
{
	emitter->MapKey("misc");
	emitter->StartMap();

	EMIT_VAR(emitter, displayFirstScreen);
	EMIT_VAR(emitter, introMovie);
	EMIT_VAR(emitter, aloneWarning);

	emitter->EndMap();
}

// player //////////////////////////////////////////////////////////////////////

void Config::cfg_player_t::Load(yaml::MapNode *root)
{
	if (root == NULL) return;

	READ_STRING(root, nickName);
}

void Config::cfg_player_t::Save(yaml::Emitter *emitter)
{
	emitter->MapKey("player");
	emitter->StartMap();

	EMIT_VAR(emitter, nickName);

	emitter->EndMap();
}

// net /////////////////////////////////////////////////////////////////////////

void Config::cfg_net_t::Load(yaml::MapNode *root)
{
	if (root == NULL) return;

	READ_STRING(root, mainServer);
	READ_STRING(root, updateServer);
	READ_BOOL(root, autoUpdates);
	READ_BOOL(root, logChats);
	READ_PATH(root, logChatsPath);
	READ_INT(root, udpRecvPort, 0, 65535);
	READ_INT(root, tcpRecvPort, 0, 65535);
	READ_INT(root, tcpServPort, 0, 65535);

	// As of 1.24 we support DNS lookups, so use the real server URL.
	if (mainServer == OLD_MAIN_SERVER ||
		mainServer == "http://" OLD_MAIN_SERVER)
	{
		mainServer = DEFAULT_MAIN_SERVER;
	}
}

void Config::cfg_net_t::Save(yaml::Emitter *emitter)
{
	emitter->MapKey("net");
	emitter->StartMap();

	EMIT_VAR(emitter, mainServer);
	EMIT_VAR(emitter, updateServer);
	EMIT_VAR(emitter, autoUpdates);
	EMIT_VAR(emitter, logChats);
	EMIT_VAR(emitter, logChatsPath);
	EMIT_VAR(emitter, udpRecvPort);
	EMIT_VAR(emitter, tcpRecvPort);
	EMIT_VAR(emitter, tcpServPort);

	emitter->EndMap();
}

// controls ////////////////////////////////////////////////////////////////////

void Config::cfg_controls_hash_t::Load(yaml::MapNode* root)
{
	if (root == NULL) return;

	READ_INT(root, motorOn, 0, INT_MAX);
	READ_INT(root, right, 0, INT_MAX);
	READ_INT(root, left, 0, INT_MAX);
	READ_INT(root, jump, 0, INT_MAX);
	READ_INT(root, fire, 0, INT_MAX);
	READ_INT(root, brake, 0, INT_MAX);
	READ_INT(root, weapon, 0, INT_MAX);
	READ_INT(root, lookBack, 0, INT_MAX);
}

void Config::cfg_controls_hash_t::Save(yaml::Emitter* emitter)
{
	emitter->StartMap();

	EMIT_VAR(emitter, motorOn);
	EMIT_VAR(emitter, right);
	EMIT_VAR(emitter, left);
	EMIT_VAR(emitter, jump);
	EMIT_VAR(emitter, fire);
	EMIT_VAR(emitter, brake);
	EMIT_VAR(emitter, weapon);
	EMIT_VAR(emitter, lookBack);

	emitter->EndMap();
}

Config::cfg_control_t Config::cfg_control_t::Key(int kc)
{
	Config::cfg_control_t ctl;
	ctl.inputType = OIS::OISKeyboard;
	ctl.kbdBinding = kc;
	return ctl;
}

void Config::cfg_control_t::SetKey(int kc)
{
	inputType = OIS::OISKeyboard;
	kbdBinding = kc;
}

bool Config::cfg_control_t::IsKey(int kc) const
{
	return inputType == OIS::OISKeyboard && kbdBinding == kc;
}

void Config::cfg_control_t::Load(yaml::MapNode *root)
{
	if (root == NULL) return;

	READ_INT(root, inputType, 0, 4);
	READ_INT(root, kbdBinding, 0, 65535);
	READ_INT(root, button, -32768, 32767);
	READ_INT(root, axis, 0, 65535);
	READ_INT(root, direction, -32768, 32767);
	READ_INT(root, pov, 0, 65535);
	READ_INT(root, slider, 0, 65535);
	READ_INT(root, sensitivity, -32768, 32767);
	READ_INT(root, joystickId, -32768, 32767);
}

void Config::cfg_control_t::Save(yaml::Emitter *emitter)
{
	emitter->StartMap();

	EMIT_VAR(emitter, inputType);
	EMIT_VAR(emitter, kbdBinding);
	EMIT_VAR(emitter, button);
	EMIT_VAR(emitter, axis);
	EMIT_VAR(emitter, direction);
	EMIT_VAR(emitter, pov);
	EMIT_VAR(emitter, slider);
	EMIT_VAR(emitter, sensitivity);
	EMIT_VAR(emitter, joystickId);

	emitter->EndMap();
}

}  // namespace Util
}  // namespace HoverRace
