
// Config.cpp
// Configuration system.
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

#include "StdAfx.h"

#include <limits.h>
#include <stdlib.h>

#include <string>
#include <sstream>
#include <exception>

#ifdef _WIN32
	#include <windows.h>
	#include <shlobj.h>
	#include <lmcons.h>
#else
	#include <unistd.h>
#endif

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

#include "yaml/Emitter.h"
#include "yaml/MapNode.h"
#include "yaml/ScalarNode.h"
#include "yaml/SeqNode.h"
#include "yaml/Parser.h"

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

#define DEFAULT_NICKNAME		"Player"
#define DEFAULT_MAIN_SERVER		"66.197.183.245/~sirbrock/imr/rl.php"
#define DEFAULT_UDP_RECV_PORT	9531
#define DEFAULT_TCP_RECV_PORT	9531
#define DEFAULT_TCP_SERV_PORT	9530

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

#define EMIT_VAR(emitter,name) \
	(emitter)->MapKey(#name); \
	(emitter)->Value(name);

using namespace HoverRace::Util;

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
					 bool prerelease, const std::string &path) :
	verMajor(verMajor), verMinor(verMinor), verPatch(verPatch), verBuild(verBuild),
	prerelease(prerelease)
{
	this->path = (path.length() == 0) ? GetDefaultPath() : path;

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

	// Set initial defaults.
	ResetToDefaults();
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
						   bool prerelease, const std::string &path)
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
std::string Config::GetDefaultPath()
{
#ifdef _WIN32
	char dpath[MAX_PATH] = {0};
	HRESULT hr = 
		SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, dpath);
	if (SUCCEEDED(hr)) {
		std::string retv(dpath);
		retv += "\\HoverRace.com\\HoverRace";
		return retv;
	}
	else {
		throw ConfigExn("Unable to determine configuration directory");
	}
#else
	char *home = getenv("HOME");
	if (home != NULL) {
		std::string retv(home);
		retv += "/.hoverrace";
		return retv;
	}
	else {
		throw ConfigExn(
			"Unable to determine configuration directory "
			"(HOME environment variable not set)");
	}
#endif
}

std::string Config::GetConfigFilename() const
{
	std::string retv(path);
	retv += DIRSEP CONFIG_FILENAME;
	return retv;
}

/**
 * Retrieve the directory for media files.
 * @return The directory path (may be relative).
 */
std::string Config::GetMediaPath() const
{
#	ifdef _WIN32
		return ".." DIRSEP "share";
#	else
		return ".." DIRSEP "share" DIRSEP PACKAGE DIRSEP;
#	endif
}

/**
 * Retrieve the path to a media file.
 * @param file The media filename (may not be blank).
 * @return The file path (may be relative).
 */
std::string Config::GetMediaPath(const std::string &file) const
{
#	ifdef _WIN32
		return (".." DIRSEP "share" DIRSEP) + file;
#	else
		return (".." DIRSEP "share" DIRSEP PACKAGE DIRSEP) + file;
#	endif
}

/**
 * Retrieve the directory for downloaded tracks.
 * @return The directory path (may be relative).
 */
std::string Config::GetTrackPath() const
{
	return path + (DIRSEP "Tracks");
}

/**
 * Retrieve the path to a downloaded track.
 * @param file The track filename (may not be blank).
 *             If the filename does not end in ".trk", it will be appended
 *             automatically.
 * @return The directory path (may be relative).
 */
std::string Config::GetTrackPath(const std::string &file) const
{
	std::string retv(path);
	retv += (DIRSEP "Tracks" DIRSEP);
	retv += file;
	if (file.length() < 4 || file.rfind(".trk") != (file.length() - 4)) {
		retv += ".trk";
	}
	return retv;
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
	net.udpRecvPort = DEFAULT_UDP_RECV_PORT;
	net.tcpRecvPort = DEFAULT_TCP_RECV_PORT;
	net.tcpServPort = DEFAULT_TCP_SERV_PORT;

	// Default controls.
	controls[0].motorOn = 1;
	controls[0].right = 5;
	controls[0].left = 6;
	controls[0].jump = 3;
	controls[0].fire = 2;
	controls[0].brake = 4;
	controls[0].weapon = 11;
	controls[0].lookBack = 10;

	controls[1].motorOn = 66;
	controls[1].right = 64;
	controls[1].left = 61;
	controls[1].jump = 83;
	controls[1].fire = 78;
	controls[1].brake = 79;
	controls[1].weapon = 77;
	controls[1].lookBack = 65;

	memset(&controls[2], 0, sizeof(cfg_controls_t));
	memset(&controls[3], 0, sizeof(cfg_controls_t));
}

/**
 * Load the configuration.
 * Does nothing if the configuration file doesn't exist.
 * @throws ConfigExn If an error occurs while reading.
 */
void Config::Load()
{
	const std::string &cfgfile = GetConfigFilename();
	
	FILE *in = fopen(cfgfile.c_str(), "rb");
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
			video.Load(dynamic_cast<yaml::MapNode*>(root->Get("video")));
			audio.Load(dynamic_cast<yaml::MapNode*>(root->Get("audio")));
			misc.Load(dynamic_cast<yaml::MapNode*>(root->Get("misc")));
			player.Load(dynamic_cast<yaml::MapNode*>(root->Get("player")));
			net.Load(dynamic_cast<yaml::MapNode*>(root->Get("net")));

			// Get the controls.
			yaml::SeqNode *ctlseq = dynamic_cast<yaml::SeqNode*>(root->Get("controls"));
			if (ctlseq != NULL) {
				int i = 0;
				yaml::SeqNode::children_t *children = ctlseq->GetChildren();
				for (yaml::SeqNode::children_t::iterator iter = children->begin();
					iter != children->end() && i < MAX_PLAYERS; ++iter, ++i)
				{
					controls[i].Load(dynamic_cast<yaml::MapNode*>(*iter), i);
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
	const std::string &cfgfile = GetConfigFilename();

	// Create the config directory.
	fs::path dirpath(path);
	if (!fs::exists(dirpath)) {
		if (!fs::create_directories(dirpath)) {
			throw ConfigExn(
				(std::string("Unable to create directory: ") + path).c_str());
		}
	}

	FILE *out = fopen(cfgfile.c_str(), "wb");
	if(out == NULL) {
		throw ConfigExn(
			(std::string("Unable to create configuration file: ") + cfgfile).c_str());
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
		emitter->MapKey("controls");
		emitter->StartSeq();
		for (int i = 0; i < MAX_PLAYERS; ++i) {
			controls[i].Save(emitter);
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
}

void Config::cfg_misc_t::Save(yaml::Emitter *emitter)
{
	emitter->MapKey("misc");
	emitter->StartMap();

	EMIT_VAR(emitter, displayFirstScreen);
	EMIT_VAR(emitter, introMovie);

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
	READ_INT(root, udpRecvPort, 0, 65535);
	READ_INT(root, tcpRecvPort, 0, 65535);
	READ_INT(root, tcpServPort, 0, 65535);
}

void Config::cfg_net_t::Save(yaml::Emitter *emitter)
{
	emitter->MapKey("net");
	emitter->StartMap();

	EMIT_VAR(emitter, mainServer);
	EMIT_VAR(emitter, udpRecvPort);
	EMIT_VAR(emitter, tcpRecvPort);
	EMIT_VAR(emitter, tcpServPort);

	emitter->EndMap();
}

// controls ////////////////////////////////////////////////////////////////////

void Config::cfg_controls_t::Load(yaml::MapNode *root, int playerNum)
{
	if (root == NULL) return;

	// The first player is limited to a certain set of keys for some reason.
	int max = (playerNum == 0) ? 59 : 86;

	READ_INT(root, motorOn, 0, max);
	READ_INT(root, right, 0, max);
	READ_INT(root, left, 0, max);
	READ_INT(root, jump, 0, max);
	READ_INT(root, fire, 0, max);
	READ_INT(root, brake, 0, max);
	READ_INT(root, weapon, 0, max);
	READ_INT(root, lookBack, 0, max);
}

void Config::cfg_controls_t::Save(yaml::Emitter *emitter)
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
