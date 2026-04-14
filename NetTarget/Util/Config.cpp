
/* Config.cpp
	Global configuration reader and writer. */

#include "stdafx.h"

#include <stdlib.h>

#include <string>
#include <exception>

#ifdef _WIN32
	#include <windows.h>
	#include <shlobj.h>
#endif

#include "yaml/Emitter.h"
#include "yaml/MapNode.h"
#include "yaml/ScalarNode.h"
#include "yaml/SeqNode.h"
#include "yaml/Parser.h"

#include "Config.h"

#ifdef _WIN32
	#define DIRSEP "\\"
#else
	#define DIRSEP "/"
#endif
#define CONFIG_FILENAME			"config.yml"

#define DEFAULT_NICKNAME		"Player"
#define DEFAULT_MAIN_SERVER		"www.hoverrace.com/imr/rl.php"
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

MR_Config *MR_Config::instance = NULL;

/**
 * Create a new instance using the specified directory.
 * @param version The app version.
 * @param path (Optional) The directory for storing configuration data.
 *             The default is to use {@link #GetDefaultPath()}.
 */
MR_Config::MR_Config(const std::string &version, const std::string &path) :
	version(version)
{
	this->path = (path.length() == 0) ? GetDefaultPath() : path;

	// Set initial defaults.
	ResetToDefaults();
}

MR_Config::~MR_Config()
{
}


/**
 * Initialize the singleton instance.
 * @param version The app version.
 * @param path (Optional) The directory for storing configuration data.
 *             The default is to use {@link #GetDefaultPath()}.
 * @return The config instance.
 */
MR_Config *MR_Config::Init(const std::string &version, const std::string &path)
{
	if (instance == NULL) {
		instance = new MR_Config(version, path);
	}
	return instance;
}

/**
 * Destroy the singleton instance.
 */
void MR_Config::Shutdown()
{
	if (instance != NULL) {
		delete instance;
		instance = NULL;
	}
}

/**
 * Retrieve the OS-specific default configuration directory.
 * @return The fully-qualified path.
 * @throws std::exception if unable to retrieve the default directory.
 */
std::string MR_Config::GetDefaultPath()
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
		throw std::exception("Unable to determine configuration directory");
	}
#else
	char *home = getenv("HOME");
	if (home != NULL) {
		std::string retv(home);
		retv += "/.hoverrace";
		return retv;
	}
	else {
		throw std::exception(
			"Unable to determine configuration directory "
			"(HOME environment variable not set)");
	}
#endif
}

std::string MR_Config::GetConfigFilename() const
{
	std::string retv(path);
	retv += DIRSEP CONFIG_FILENAME;
	return retv;
}

/**
 * Retrieve the directory for media files.
 * @return The directory path (may be relative).
 */
std::string MR_Config::GetMediaPath() const
{
	return ".." DIRSEP "share";
}

/**
 * Retrieve the path to a media file.
 * @param file The media filename (may not be blank).
 * @return The file path (may be relative).
 */
std::string MR_Config::GetMediaPath(const std::string &file) const
{
	return (".." DIRSEP "share" DIRSEP) + file;
}

/**
 * Retrieve the directory for downloaded tracks.
 * @return The directory path (may be relative).
 */
std::string MR_Config::GetTrackPath() const
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
std::string MR_Config::GetTrackPath(const std::string &file) const
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
void MR_Config::ResetToDefaults()
{
	video.gamma = 1.2;
	video.contrast = 0.95;
	video.brightness = 0.95;
	video.nativeBppFullscreen = true;
	video.useOriginalCameraParams = false;
	video.adaptiveRenderScale = true;
	video.adaptiveRenderScaleMinPercent = 50;

	video.windowMonitor = "";
	video.windowMonitorPosX = 0;
	video.windowMonitorPosY = 0;
	video.windowPosX = CW_USEDEFAULT;
	video.windowPosY = CW_USEDEFAULT;
	video.windowSizeX = GetSystemMetrics(SM_CXSCREEN) / 2;
	video.windowSizeY = GetSystemMetrics(SM_CYSCREEN) / 2;
	video.windowMaximized = false;

	audio.sfxVolume = 0.2;

	misc.displayFirstScreen = true;
	misc.introMovie = true;
	misc.mainLoopSleepLength = 0;
	misc.directConnect = false;

	player.nickName = DEFAULT_NICKNAME;
	player.nickNameSet = false;
	player.onlinePartySize = 1;
	player.splitScreenPartySize = 2;
	for(int i = 0; i < MR_MAX_LOCAL_PLAYER; ++i) {
		player.onlinePartyNames[i] = "";
	}

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

	for(int i = 2; i < MAX_PLAYERS; ++i) {
		memset(&controls[i], 0, sizeof(cfg_controls_t));
	}

}

/**
 * Load the configuration.
 * Does nothing if the configuration file doesn't exist.
 * @throws std::exception If an error occurs while reading.
 */
void MR_Config::Load()
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
		throw std::exception(ex);
	}

	fclose(in);
}

/**
 * Save the configuration.
 * @throws std::exception If an error occurs while saving.
 */
void MR_Config::Save()
{
	const std::string &cfgfile = GetConfigFilename();

	// Create the config directory.
#ifdef _WIN32
	int cdr = SHCreateDirectoryEx(NULL, path.c_str(), NULL);
	if(cdr != ERROR_SUCCESS) {
		if(cdr != ERROR_FILE_EXISTS && cdr != ERROR_ALREADY_EXISTS) {
			throw std::exception(
				(std::string("Unable to create directory: ") + path).c_str());
		}
	}
#else
	//TODO: Unimplemented.
	abort();
#endif

	FILE *out = fopen(cfgfile.c_str(), "wb");
	if(out == NULL) {
		throw std::exception(
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

void MR_Config::SaveVersion(yaml::Emitter *emitter)
{
	EMIT_VAR(emitter, version);
}

// video ///////////////////////////////////////////////////////////////////////

void MR_Config::cfg_video_t::Load(yaml::MapNode *root)
{
	if (root == NULL) return;

	READ_DOUBLE(root, gamma, 0.0, 2.0);
	READ_DOUBLE(root, contrast, 0.0, 1.0);
	READ_DOUBLE(root, brightness, 0.0, 1.0);

	READ_BOOL(root, nativeBppFullscreen);
	READ_BOOL(root, useOriginalCameraParams);
	READ_BOOL(root, adaptiveRenderScale);
	READ_INT(root, adaptiveRenderScaleMinPercent, 10, 100);
	READ_STRING(root, windowMonitor);
	READ_INT(root, windowMonitorPosX, INT_MIN, INT_MAX);
	READ_INT(root, windowMonitorPosY, INT_MIN, INT_MAX);

	{
		yaml::ScalarNode *_scalar = dynamic_cast<yaml::ScalarNode*>(root->Get("windowPosX"));
		if (_scalar != NULL) windowPosX = _scalar->AsInt(windowPosX);
	}
	{
		yaml::ScalarNode *_scalar = dynamic_cast<yaml::ScalarNode*>(root->Get("windowPosY"));
		if (_scalar != NULL) windowPosY = _scalar->AsInt(windowPosY);
	}

	{
		yaml::ScalarNode *_scalar = dynamic_cast<yaml::ScalarNode*>(root->Get("windowSizeX"));
		if (_scalar != NULL) windowSizeX = _scalar->AsInt(windowSizeX, 320, INT_MAX);
	}
	{
		yaml::ScalarNode *_scalar = dynamic_cast<yaml::ScalarNode*>(root->Get("windowSizeY"));
		if (_scalar != NULL) windowSizeY = _scalar->AsInt(windowSizeY, 240, INT_MAX);
	}
	READ_BOOL(root, windowMaximized);
}

void MR_Config::cfg_video_t::Save(yaml::Emitter *emitter)
{
	emitter->MapKey("video");
	emitter->StartMap();

	EMIT_VAR(emitter, gamma);
	EMIT_VAR(emitter, contrast);
	EMIT_VAR(emitter, brightness);

	EMIT_VAR(emitter, nativeBppFullscreen);
	EMIT_VAR(emitter, useOriginalCameraParams);
	EMIT_VAR(emitter, adaptiveRenderScale);
	EMIT_VAR(emitter, adaptiveRenderScaleMinPercent);
	EMIT_VAR(emitter, windowMonitor);
	EMIT_VAR(emitter, windowMonitorPosX);
	EMIT_VAR(emitter, windowMonitorPosY);

	EMIT_VAR(emitter, windowPosX);
	EMIT_VAR(emitter, windowPosY);

	EMIT_VAR(emitter, windowSizeX);
	EMIT_VAR(emitter, windowSizeY);
	EMIT_VAR(emitter, windowMaximized);

	emitter->EndMap();
}

// audio ///////////////////////////////////////////////////////////////////////

void MR_Config::cfg_audio_t::Load(yaml::MapNode *root)
{
	if (root == NULL) return;

	READ_FLOAT(root, sfxVolume, 0.0f, 1.0f);
}

void MR_Config::cfg_audio_t::Save(yaml::Emitter *emitter)
{
	emitter->MapKey("audio");
	emitter->StartMap();

	EMIT_VAR(emitter, sfxVolume);

	emitter->EndMap();
}

// misc ////////////////////////////////////////////////////////////////////////

void MR_Config::cfg_misc_t::Load(yaml::MapNode *root)
{
	if (root == NULL) return;

	READ_BOOL(root, displayFirstScreen);
	READ_BOOL(root, introMovie);
	READ_INT(root, mainLoopSleepLength, 0, 0);
	READ_BOOL(root, directConnect);
}

void MR_Config::cfg_misc_t::Save(yaml::Emitter *emitter)
{
	emitter->MapKey("misc");
	emitter->StartMap();

	EMIT_VAR(emitter, displayFirstScreen);
	EMIT_VAR(emitter, introMovie);
	EMIT_VAR(emitter, mainLoopSleepLength);
	EMIT_VAR(emitter, directConnect);

	emitter->EndMap();
}

// player //////////////////////////////////////////////////////////////////////

void MR_Config::cfg_player_t::Load(yaml::MapNode *root)
{
	if (root == NULL) return;

	yaml::ScalarNode *scalar = dynamic_cast<yaml::ScalarNode*>(root->Get("nickName"));
	if (scalar != NULL) {
		nickName = scalar->AsString();
		nickNameSet = !nickName.empty();
	}

	READ_INT(root, onlinePartySize, 1, MR_MAX_LOCAL_PLAYER);
	READ_INT(root, splitScreenPartySize, 2, MR_MAX_LOCAL_PLAYER);

	bool hasPartyNames = false;
	yaml::SeqNode *partySeq = dynamic_cast<yaml::SeqNode*>(root->Get("onlinePartyNames"));
	if (partySeq != NULL) {
		hasPartyNames = true;
		int i = 0;
		yaml::SeqNode::children_t *children = partySeq->GetChildren();
		for(yaml::SeqNode::children_t::iterator iter = children->begin();
			iter != children->end() && i < MR_MAX_LOCAL_PLAYER; ++iter, ++i)
		{
			yaml::ScalarNode *child = dynamic_cast<yaml::ScalarNode*>(*iter);
			onlinePartyNames[i] = (child != NULL) ? child->AsString() : "";
		}
		for(; i < MR_MAX_LOCAL_PLAYER; ++i) {
			onlinePartyNames[i] = "";
		}
	}

	if(!hasPartyNames) {
		for(int i = 0; i < MR_MAX_LOCAL_PLAYER; ++i) {
			onlinePartyNames[i] = "";
		}
		onlinePartySize = 1;
	}

	splitScreenPartySize = max(2, min(splitScreenPartySize,
		MR_MAX_LOCAL_PLAYER));

	if(onlinePartyNames[0].empty()) {
		onlinePartyNames[0] = nickName.empty() ? DEFAULT_NICKNAME : nickName;
	}
}

void MR_Config::cfg_player_t::Save(yaml::Emitter *emitter)
{
	emitter->MapKey("player");
	emitter->StartMap();

	EMIT_VAR(emitter, nickName);
	EMIT_VAR(emitter, onlinePartySize);
	EMIT_VAR(emitter, splitScreenPartySize);
	emitter->MapKey("onlinePartyNames");
	emitter->StartSeq();
	for(int i = 0; i < MR_MAX_LOCAL_PLAYER; ++i) {
		emitter->Value(onlinePartyNames[i]);
	}
	emitter->EndSeq();

	emitter->EndMap();
}

// net /////////////////////////////////////////////////////////////////////////

void MR_Config::cfg_net_t::Load(yaml::MapNode *root)
{
	if (root == NULL) return;

	READ_STRING(root, mainServer);
	READ_INT(root, udpRecvPort, 0, 65535);
	READ_INT(root, tcpRecvPort, 0, 65535);
	READ_INT(root, tcpServPort, 0, 65535);
}

void MR_Config::cfg_net_t::Save(yaml::Emitter *emitter)
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

void MR_Config::cfg_controls_t::Load(yaml::MapNode *root, int playerNum)
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

void MR_Config::cfg_controls_t::Save(yaml::Emitter *emitter)
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
