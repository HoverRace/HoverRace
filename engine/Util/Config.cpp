
// Config.cpp
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

#include <limits.h>
#include <stdlib.h>

#include <iomanip>
#include <string>
#include <sstream>
#include <exception>

#ifdef _WIN32
#	include <windows.h>
#	include <comdef.h>
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

#include <SDL2/SDL_keycode.h>

#include "yaml/Emitter.h"
#include "yaml/MapNode.h"
#include "yaml/ScalarNode.h"
#include "yaml/SeqNode.h"
#include "yaml/Parser.h"

#include "../Control/Controller.h"
#include "../Parcel/Bundle.h"
#include "../Parcel/ResBundle.h"
#include "../Parcel/TrackBundle.h"
#include "Locale.h"
#include "Log.h"
#include "Str.h"

#include "Config.h"

namespace fs = boost::filesystem;
using HoverRace::Control::InputEventController;

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

#define READ_BOOL(root, name) \
	root->ReadBool(#name, name)

#define READ_INT(root, name, min, max) \
	root->ReadInt(#name, name, min, max)

#define READ_FLOAT(root, name, min, max) \
	root->ReadFloat(#name, name, min, max)

#define READ_DOUBLE(root, name, min, max) \
	root->ReadDouble(#name, name, min, max)

#define READ_STRING(root, name) \
	root->ReadString(#name, name)

#define READ_PATH(root, name) \
	root->ReadPath(#name, name)

#define EMIT_VAR(emitter, name) \
	(emitter)->MapKey(#name); \
	(emitter)->Value(name);

namespace HoverRace {
namespace Util {

namespace {

/**
 * Find the user's OS-specific "personal" data directory.
 *
 * On Windows, this is the "My Documents" folder.
 * Everywhere else, this is the user's home directory.
 *
 * @return The directory path (empty if the directory couldn't be determined).
 */
OS::path_t FindPersonalDir()
{
	// Only log the warning once.
	static bool loggedWarning = false;

#	ifdef _WIN32
		wchar_t *dpath;
		HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents,
			0, nullptr, &dpath);
		if (SUCCEEDED(hr)) {
			auto retv = OS::path_t(dpath);
			CoTaskMemFree(dpath);
			return retv;
		}
		else {
			if (!loggedWarning) {
				HR_LOG(warning) <<
					"Unable to look up the \"My Documents\" folder: " <<
					_com_error(hr).ErrorMessage();
				loggedWarning = true;
			}
			return OS::path_t();
		}
#	else
		char *home = getenv("HOME");
		if (home) {
			return OS::path_t(home);
		}
		else {
			if (!loggedWarning) {
				HR_LOG(warning) << "Unable to look up the home directory.";
				loggedWarning = true;
			}
			return OS::path_t();
		}
#	endif
}

}  // namespace

const std::string Config::TRACK_EXT(".trk");

std::unique_ptr<Config> Config::instance{};

/**
 * Create a new instance using the specified directory.
 * @param packageName The name of the application for shared data files.
 * @param verMajor The major (first) component of the version number.
 * @param verMinor The minor (second) component of the version number.
 * @param verPatch The patch (third) component of the version number.
 * @param verBuild The build (fourth) component of the version number.
 * @param prerelease Whether this build is a development version.
 * @param mediaPath (Optional) The directory for finding media files.
 *                  The default is to use {@link #GetDefaultMediaPath()}.
 * @param sysCfgPath (Optional) The directory to use for the system config file.
 *                   The default is to search a platform-specific list of
 *                   directories.
 * @param path (Optional) The directory for storing configuration data.
 *             The default is to use {@link #GetBaseDataPath()} and
 *             {@link #GetBaseConfigPath()}.
 */
Config::Config(const std::string &packageName,
	int verMajor, int verMinor, int verPatch, int verBuild,
	bool prerelease, const OS::path_t &mediaPath,
	const OS::path_t &sysCfgPath,
	const OS::path_t &path) :
	packageName(packageName),
	unlinked(false),
	sysCfgPath(sysCfgPath),
	verBuild(verBuild),
	prerelease(prerelease)
{
#	ifndef _WIN32
		// Use the XDG Base Directory specification (if possible) to determine
		// where to place user data.
		xdg = (xdgHandle*)malloc(sizeof(xdgHandle));
		if (!xdgInitHandle(xdg)) {
			free(xdg);
			xdg = NULL;

			HR_LOG(warning) <<
				"Unable to use XDG directories for configuration; "
				"falling back to default.";
		}
#	endif

	LoadSystem();

	// If the override path was specified, use that for both data and config.
	this->dataPath = path.empty() ? GetBaseDataPath() : path;
	this->cfgPath = path.empty() ? GetBaseConfigPath() : path;
	this->mediaPath = mediaPath.empty() ? GetDefaultMediaPath() : mediaPath;

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

	defaultFontName = "freefont/FreeSans";
	defaultMonospaceFontName = "freefont/FreeMono";
	defaultSymbolFontName = "fontawesome/fontawesome-webfont";

	locale.reset(new Locale(app.localePath, packageName));

	// Set initial defaults.
	ResetToDefaults();

	OS::path_t resDirName = Str::UP("res");
	resBundle =
		std::make_shared<Parcel::ResBundle>(this->mediaPath / resDirName);

	OS::path_t tracksDirName = Str::UP("tracks");

	userTrackPath = this->dataPath / tracksDirName;

	auto mediaTrackBundle =
		std::make_shared<Parcel::Bundle>(this->mediaPath / tracksDirName);
#	ifdef _WIN32
		trackBundle = std::make_shared<Parcel::TrackBundle>(userTrackPath,
			std::make_shared<Parcel::Bundle>(Str::UP("track_source"),  // Historical.
			std::make_shared<Parcel::Bundle>(tracksDirName,  // Historical.
			mediaTrackBundle)));
#	else
		trackBundle = std::make_shared<Parcel::TrackBundle>(userTrackPath,
			mediaTrackBundle);
#	endif
}

Config::~Config()
{
#	ifndef _WIN32
		if (xdg != NULL) {
			xdgWipeHandle(xdg);
			free(xdg);
		}
#	endif
}


/**
 * Initialize the singleton instance.
 * @param packageName The name of the application for shared data files.
 * @param verMajor The major (first) component of the version number.
 * @param verMinor The minor (second) component of the version number.
 * @param verPatch The patch (third) component of the version number.
 * @param verBuild The build (fourth) component of the version number.
 * @param prerelease Whether this build is a development version.
 * @param mediaPath (Optional) The directory for finding media files.
 *                  The default is to use {@link #GetDefaultMediaPath()}.
 * @param sysCfgPath (Optional) The directory to use for the system config file.
 *                   The default is to search a platform-specific list of
 *                   directories.
 * @param path (Optional) The directory for storing configuration data.
 *             The default is to use {@link #GetBaseDataPath()} and
 *             {@link #GetBaseConfigPath()}.
 * @return The config instance.
 */
Config *Config::Init(const std::string &packageName,
	int verMajor, int verMinor, int verPatch, int verBuild,
	bool prerelease, const OS::path_t &mediaPath,
	const OS::path_t &sysCfgPath, const OS::path_t &path)
{
	if (!instance) {
		instance.reset(
			new Config(packageName, verMajor, verMinor, verPatch, verBuild,
				prerelease, mediaPath, sysCfgPath, path));
	}
	return instance.get();
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
 * Determine the OS-specific default configuration directory.
 * Note: This is the top-level directory for per-user data, without our
 * package namespace.
 * @return The fully-qualified path.
 * @throws ConfigExn if unable to retrieve the default directory.
 */
OS::path_t Config::GetDefaultBasePath()
{
#ifdef _WIN32
	wchar_t dpath[MAX_PATH] = {0};
	HRESULT hr =
		SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, dpath);
	if (SUCCEEDED(hr)) {
		return OS::path_t(dpath);
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
		return OS::path_t(home);
	}
	else {
		throw ConfigExn(
			"Unable to determine configuration directory "
			"(HOME environment variable not set)");
	}
#endif
}

/**
 * Append our package namespace to the base configuration directory.
 * @param path The path to append the namespace onto.
 */
void Config::AppendPackageSubdir(OS::path_t &path) const
{
#	ifdef _WIN32
		path /= L"HoverRace.com";
		path /= L"HoverRace";
#	else
		if (xdg == NULL) {
			path /= ".hoverrace";
		}
		else {
			path /= "HoverRace";
		}
#	endif
}

/**
 * Determine the OS-specific private data directory.
 * @return The path to where we can store user data files.
 */
OS::path_t Config::GetBaseDataPath() const
{
#	ifdef _WIN32
		OS::path_t retv = GetDefaultBasePath();
#	else
		OS::path_t retv = (xdg == NULL) ? GetDefaultBasePath() : xdgDataHome(xdg);
#	endif
	AppendPackageSubdir(retv);
	return retv;
}

/**
 * Determine the OS-specific private configuration directory.
 * @return The path to where we can store user configuration files.
 */
OS::path_t Config::GetBaseConfigPath() const
{
#	ifdef _WIN32
		OS::path_t retv = GetDefaultBasePath();
#	else
		OS::path_t retv = (xdg == NULL) ? GetDefaultBasePath() : xdgConfigHome(xdg);
#	endif
	AppendPackageSubdir(retv);
	return retv;
}

OS::path_t Config::GetConfigFilename() const
{
	OS::path_t retv(cfgPath);
	if (prerelease) {
		retv /= Str::UP(PREREL_CONFIG_FILENAME);
	}
	else {
		retv /= Str::UP(CONFIG_FILENAME);
	}
	return retv;
}

/**
 * Retrieve the OS-specific default media path.
 * First, we look for a config file in the current directory, then in the
 * system-global directory (if applicable), then finally the relative directory.
 * @return The directory path (may be relative).
 */
OS::path_t Config::GetDefaultMediaPath()
{
	if (app.mediaPath.empty()) {
#		ifdef _WIN32
			return "../share";
#		else
			return "share";
#		endif
	}
	else {
		return app.mediaPath;
	}
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
	return mediaPath / Str::UP(file.c_str());
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
	return userTrackPath / Str::UP(filename.c_str());
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
	retv /= Str::UP("scripts");
	retv /= Str::UP("help");
	retv /= Str::UP(filename.c_str());
	return retv;
}

/**
 * Generates a new unique file for the screenshot.
 * @param ext The extension to use on the generated filename.
 * @return The generated path.  The file will exist and will be empty, ready
 *         for writing.
 * @throw Exception The path could not be generated.
 */
OS::path_t Config::GenerateScreenshotPath(const std::string &ext) const
{
	// This isn't intended to be secure; we assume that we have the destination
	// directory to ourselves, and we just want to avoid filename collisions
	// when done in rapid succession.

	OS::path_t path = misc.screenshotPath;
	if (path.empty()) {
		throw Exception("Screenshot path is not configured.");
	}
	if (!fs::exists(path)) {
		if (!fs::create_directories(path)) {
			throw Exception(
				std::string("Unable to create screenshot directory: ") +
					(const char*)Str::PU(path));
		}
	}
	else {
		if (!fs::is_directory(path)) {
			throw Exception(std::string("Screenshot path is not a directory: ") +
				(const char*)Str::PU(path));
		}
	}

	// The filename is based on the timestamp.
	std::string base = OS::FileTimeString();

	// Loop until we find a filename that doesn't exist.
	for (int i = 0;; i++) {
		std::ostringstream oss;
		oss << base;
		if (i > 0) oss << ' ' << i;
		oss << ext;

		OS::path_t retv = path / oss.str();

		//TODO: Try to open with O_CREAT and O_EXCL and check writing.

		if (!fs::exists(retv)) {
			return retv;
		}
	}
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
	OS::path_t retv = FindPersonalDir();
	if (!retv.empty()) {
		retv /= L"HoverRace Chat";
	}
	return retv;
}

/**
 * Retrieve the default path for screenshots.
 * @return The fully-qualified path (may be empty if base path could not
 *         be retrieved from the system).
 */
OS::path_t Config::GetDefaultScreenshotPath()
{
	OS::path_t retv = FindPersonalDir();
	if (!retv.empty()) {
		retv /= L"HoverRace Screenshots";
	}
	return retv;
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
 * Retrieve the default font name for monospace text (e.g. the console).
 * @return The font name (never emepty).
 */
const std::string &Config::GetDefaultMonospaceFontName() const
{
	return defaultMonospaceFontName;
}

/**
 * Retrieve the default font name for symbols.
 * @return The font name (never emepty).
 */
const std::string &Config::GetDefaultSymbolFontName() const
{
	return defaultSymbolFontName;
}

/**
 * Resets the configuration to the default "factory" settings.
 */
void Config::ResetToDefaults()
{
	video.ResetToDefaults();
	audio.ResetToDefaults();
	i18n.ResetToDefaults();
	misc.ResetToDefaults();
	player.ResetToDefaults();
	net.ResetToDefaults();

	// Default controls.
	controlsHash[0].motorOn  = InputEventController::HashKeyboardEvent(SDLK_LSHIFT);
	controlsHash[0].right    = InputEventController::HashKeyboardEvent(SDLK_RIGHT);
	controlsHash[0].left     = InputEventController::HashKeyboardEvent(SDLK_LEFT);
	controlsHash[0].jump     = InputEventController::HashKeyboardEvent(SDLK_UP);
	controlsHash[0].fire     = InputEventController::HashKeyboardEvent(SDLK_LCTRL);
	controlsHash[0].brake    = InputEventController::HashKeyboardEvent(SDLK_DOWN);
	controlsHash[0].weapon   = InputEventController::HashKeyboardEvent(SDLK_TAB);
	controlsHash[0].lookBack = InputEventController::HashKeyboardEvent(SDLK_END);

	controlsHash[1].motorOn  = InputEventController::HashKeyboardEvent(SDLK_f);
	controlsHash[1].right    = InputEventController::HashKeyboardEvent(SDLK_d);
	controlsHash[1].left     = InputEventController::HashKeyboardEvent(SDLK_a);
	controlsHash[1].jump     = InputEventController::HashKeyboardEvent(SDLK_s);
	controlsHash[1].fire     = InputEventController::HashKeyboardEvent(SDLK_r);
	controlsHash[1].brake    = InputEventController::HashKeyboardEvent(SDLK_w);
	controlsHash[1].weapon   = InputEventController::HashKeyboardEvent(SDLK_q);
	controlsHash[1].lookBack = InputEventController::HashKeyboardEvent(SDLK_i);

	// set these to disabled (0x0000<code>)
	controlsHash[2].motorOn  = 0;
	controlsHash[2].right    = 1;
	controlsHash[2].left     = 2;
	controlsHash[2].jump     = 3;
	controlsHash[2].fire     = 4;
	controlsHash[2].brake    = 5;
	controlsHash[2].weapon   = 6;
	controlsHash[2].lookBack = 7;

	controlsHash[3].motorOn  = 8;
	controlsHash[3].right    = 9;
	controlsHash[3].left     = 10;
	controlsHash[3].jump     = 11;
	controlsHash[3].fire     = 12;
	controlsHash[3].brake    = 13;
	controlsHash[3].weapon   = 14;
	controlsHash[3].lookBack = 15;

	cameraHash.zoomIn  = InputEventController::HashKeyboardEvent(SDLK_INSERT);
	cameraHash.zoomOut = InputEventController::HashKeyboardEvent(SDLK_DELETE);
	cameraHash.panUp   = InputEventController::HashKeyboardEvent(SDLK_PAGEUP);
	cameraHash.panDown = InputEventController::HashKeyboardEvent(SDLK_PAGEDOWN);
	cameraHash.reset   = InputEventController::HashKeyboardEvent(SDLK_HOME);

	ui.consoleToggle = InputEventController::HashKeyboardEvent(SDLK_F11);
	ui.consoleUp     = InputEventController::HashKeyboardEvent(SDLK_PAGEUP);
	ui.consoleDown   = InputEventController::HashKeyboardEvent(SDLK_PAGEDOWN);
	ui.consoleTop    = InputEventController::HashKeyboardEvent(SDLK_HOME);
	ui.consoleBottom = InputEventController::HashKeyboardEvent(SDLK_END);
	ui.consolePrevCmd = InputEventController::HashKeyboardEvent(SDLK_UP);
	ui.consoleNextCmd = InputEventController::HashKeyboardEvent(SDLK_DOWN);
	ui.consoleHelp   = InputEventController::HashKeyboardEvent(SDLK_F1);

	ui.menuOk = InputEventController::HashKeyboardEvent(SDLK_RETURN);
	ui.menuCancel = InputEventController::HashKeyboardEvent(SDLK_ESCAPE);
	ui.menuExtra = InputEventController::HashKeyboardEvent(SDLK_BACKSPACE);
	ui.menuUp = InputEventController::HashKeyboardEvent(SDLK_UP);
	ui.menuDown = InputEventController::HashKeyboardEvent(SDLK_DOWN);
	ui.menuLeft = InputEventController::HashKeyboardEvent(SDLK_LEFT);
	ui.menuRight = InputEventController::HashKeyboardEvent(SDLK_RIGHT);
	ui.menuNext = InputEventController::HashKeyboardEvent(SDLK_TAB);
	ui.menuPrev = InputEventController::HashKeyboardEvent(SDLK_SELECT);

	runtime.silent = false;
	runtime.showFramerate = false;
	runtime.enableConsole = true;
	runtime.enableDebugOverlay = false;
	runtime.enableHud = true;
	runtime.skipStartupWarning = false;
	runtime.profiling = false;
}

void Config::LoadSystem()
{
	if (sysCfgPath.empty()) {
#		ifdef _WIN32
			LoadSystem(Str::UP("../../etc/" CONFIG_FILENAME));
#		else
			LoadSystem(Str::UP("/etc/hoverrace/" CONFIG_FILENAME));
			LoadSystem(Str::UP("../etc/" CONFIG_FILENAME));
			LoadSystem(Str::UP("etc/" CONFIG_FILENAME));
#		endif
	}
	else {
		LoadSystem(sysCfgPath / Str::UP(CONFIG_FILENAME));
	}
}

void Config::LoadSystem(const OS::path_t &path)
{
	FILE *in = OS::FOpen(path, "rb");
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
			app.Load(dynamic_cast<yaml::MapNode*>(root->Get("app")));
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
			i18n.Load(dynamic_cast<yaml::MapNode*>(root->Get("i18n")));
			misc.Load(dynamic_cast<yaml::MapNode*>(root->Get("misc")));
			player.Load(dynamic_cast<yaml::MapNode*>(root->Get("player")));
			net.Load(dynamic_cast<yaml::MapNode*>(root->Get("net")));

			// Get the controls.
			yaml::SeqNode *ctlseqh = dynamic_cast<yaml::SeqNode*>(root->Get("controls_hash"));
			if (ctlseqh != NULL) {
				int i = 0;
				for (yaml::Node *ynode : *ctlseqh) {
					controlsHash[i++].Load(dynamic_cast<yaml::MapNode*>(ynode));
				}
			}

			cameraHash.Load(dynamic_cast<yaml::MapNode*>(root->Get("camera_hash")));
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
void Config::Save() const
{
	if (unlinked) return;

	const OS::path_t cfgfile(GetConfigFilename());

	// Create the config directory.
	if (!fs::exists(cfgPath)) {
		if (!fs::create_directories(cfgPath)) {
			std::string msg(_("Unable to create directory"));
			msg += ": ";
			msg += (const char*)Str::PU(cfgPath);
			throw ConfigExn(msg.c_str());
		}
	}

	FILE *out = OS::FOpen(cfgfile, "wb");
	if (out == NULL) {
		std::string msg(_("Unable to create configuration file"));
		msg += ": ";
		msg += (const char*)Str::PU(cfgfile);
		throw ConfigExn(msg.c_str());
	}

	yaml::Emitter *emitter = NULL;
	try {
		emitter = new yaml::Emitter(out);
		emitter->StartMap();

		SaveVersion(emitter);
		video.Save(emitter);
		audio.Save(emitter);
		i18n.Save(emitter);
		misc.Save(emitter);
		player.Save(emitter);
		net.Save(emitter);

		// Save list of controls.
		emitter->MapKey("controls_hash");
		emitter->StartSeq();
		for (int i = 0; i < MAX_PLAYERS; ++i) {
			controlsHash[i].Save(emitter);
		}
		emitter->EndSeq();

		cameraHash.Save(emitter);
		ui.Save(emitter);

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

void Config::SaveVersion(yaml::Emitter *emitter) const
{
	const auto &version = shortVersion;
	EMIT_VAR(emitter, version);
}

// app /////////////////////////////////////////////////////////////////////////

void Config::app_t::Load(yaml::MapNode *root)
{
	if (root == NULL) return;

	std::string mediaPath;
	READ_STRING(root, mediaPath);
	if (!mediaPath.empty()) {
		this->mediaPath = Str::UP(mediaPath);
	}

	std::string localePath;
	READ_STRING(root, localePath);
	if (!localePath.empty()) {
		this->localePath = Str::UP(localePath);
	}
}

// video ///////////////////////////////////////////////////////////////////////

void Config::video_t::ResetToDefaults()
{
	gamma = 1.2;
	contrast = 0.95;
	brightness = 0.95;
	textScale = 0.7;
	xPos = 150;
	yPos = 150;
	xRes = 800;
	yRes = 450;

	// Use this as a sensible default.
	// We used to try to detect the current screen resolution, but we
	// initialize these defaults before the SDL video subsystem is initialized.
	fullscreen = false;
	fullscreenMonitorIndex = 0;
	xResFullscreen = 1280;
	yResFullscreen = 720;
	fullscreenRefreshRate = 0;

	stackedSplitscreen = true;
}

void Config::video_t::Load(yaml::MapNode *root)
{
	if (root == NULL) return;

	READ_DOUBLE(root, gamma, 0.0, 2.0);
	READ_DOUBLE(root, contrast, 0.0, 1.0);
	READ_DOUBLE(root, brightness, 0.0, 1.0);

	READ_DOUBLE(root, textScale, 0.5, 1.5);

	READ_INT(root, xPos, -32768, 32768);
	READ_INT(root, yPos, -32768, 32768);
	READ_INT(root, xRes, 0, 32768);
	READ_INT(root, yRes, 0, 32768);
	READ_BOOL(root, fullscreen);
	READ_INT(root, fullscreenMonitorIndex, 0, 15);
	READ_INT(root, xResFullscreen, 0, 32768);
	READ_INT(root, yResFullscreen, 0, 32768);
	READ_INT(root, fullscreenRefreshRate, 0, 32768);

	READ_BOOL(root, stackedSplitscreen);
}

void Config::video_t::Save(yaml::Emitter *emitter) const
{
	emitter->MapKey("video");
	emitter->StartMap();

	EMIT_VAR(emitter, gamma);
	EMIT_VAR(emitter, contrast);
	EMIT_VAR(emitter, brightness);

	EMIT_VAR(emitter, textScale);

	EMIT_VAR(emitter, xPos);
	EMIT_VAR(emitter, yPos);
	EMIT_VAR(emitter, xRes);
	EMIT_VAR(emitter, yRes);
	EMIT_VAR(emitter, fullscreen);
	EMIT_VAR(emitter, fullscreenMonitorIndex);
	EMIT_VAR(emitter, xResFullscreen);
	EMIT_VAR(emitter, yResFullscreen);
	EMIT_VAR(emitter, fullscreenRefreshRate);

	EMIT_VAR(emitter, stackedSplitscreen);

	emitter->EndMap();
}

// audio ///////////////////////////////////////////////////////////////////////

void Config::audio_t::ResetToDefaults()
{
	sfxVolume = 1.0;
}

void Config::audio_t::Load(yaml::MapNode *root)
{
	if (root == NULL) return;

	READ_DOUBLE(root, sfxVolume, 0.0f, 1.0f);
}

void Config::audio_t::Save(yaml::Emitter *emitter) const
{
	emitter->MapKey("audio");
	emitter->StartMap();

	EMIT_VAR(emitter, sfxVolume);

	emitter->EndMap();
}

// i18n ////////////////////////////////////////////////////////////////////////

void Config::i18n_t::ResetToDefaults()
{
	preferredLocale.clear();  // Auto-detect.
}

void Config::i18n_t::Load(yaml::MapNode *root)
{
	if (!root) return;

	READ_STRING(root, preferredLocale);
}

void Config::i18n_t::Save(yaml::Emitter *emitter) const
{
	emitter->MapKey("i18n");
	emitter->StartMap();

	EMIT_VAR(emitter, preferredLocale);

	emitter->EndMap();
}

// misc ////////////////////////////////////////////////////////////////////////

void Config::misc_t::ResetToDefaults()
{
	screenshotPath = GetDefaultScreenshotPath();
}

void Config::misc_t::Load(yaml::MapNode *root)
{
	if (root == NULL) return;

	READ_PATH(root, screenshotPath);
}

void Config::misc_t::Save(yaml::Emitter *emitter) const
{
	emitter->MapKey("misc");
	emitter->StartMap();

	EMIT_VAR(emitter, screenshotPath);

	emitter->EndMap();
}

// player //////////////////////////////////////////////////////////////////////

void Config::player_t::ResetToDefaults()
{
	// Get current user name as default nickname.
#ifdef _WIN32
	char buf[UNLEN + 1];
	DWORD bsize = sizeof(buf);
	//TODO: Handle Unicode names.
	if (GetUserName(buf, &bsize)) {
		buf[UNLEN] = '\0';
		nickName = buf;
	}
	else {
		nickName = DEFAULT_NICKNAME;
	}
#else
	char buf[LOGIN_NAME_MAX];
	if (getlogin_r(buf, sizeof(buf)) == 0) {
		nickName = buf;
	}
	else {
		nickName = DEFAULT_NICKNAME;
	}
#endif
}

void Config::player_t::Load(yaml::MapNode *root)
{
	if (root == NULL) return;

	READ_STRING(root, nickName);
}

void Config::player_t::Save(yaml::Emitter *emitter) const
{
	emitter->MapKey("player");
	emitter->StartMap();

	EMIT_VAR(emitter, nickName);

	emitter->EndMap();
}

// net /////////////////////////////////////////////////////////////////////////

void Config::net_t::ResetToDefaults()
{
	mainServer = DEFAULT_MAIN_SERVER;
	updateServer = DEFAULT_UPDATE_SERVER;
	autoUpdates = true;
	messageReceivedSound = true;
	messageReceivedSoundOnlyBg = true;
	logChats = false;
	logChatsPath = GetDefaultChatLogPath();
	udpRecvPort = net_t::DEFAULT_UDP_RECV_PORT;
	tcpRecvPort = net_t::DEFAULT_TCP_RECV_PORT;
	tcpServPort = net_t::DEFAULT_TCP_SERV_PORT;
}

void Config::net_t::Load(yaml::MapNode *root)
{
	if (root == NULL) return;

	READ_STRING(root, mainServer);
	READ_STRING(root, updateServer);
	READ_BOOL(root, autoUpdates);
	READ_BOOL(root, messageReceivedSound);
	READ_BOOL(root, messageReceivedSoundOnlyBg);
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

void Config::net_t::Save(yaml::Emitter *emitter) const
{
	emitter->MapKey("net");
	emitter->StartMap();

	EMIT_VAR(emitter, mainServer);
	EMIT_VAR(emitter, updateServer);
	EMIT_VAR(emitter, autoUpdates);
	EMIT_VAR(emitter, messageReceivedSound);
	EMIT_VAR(emitter, messageReceivedSoundOnlyBg);
	EMIT_VAR(emitter, logChats);
	EMIT_VAR(emitter, logChatsPath);
	EMIT_VAR(emitter, udpRecvPort);
	EMIT_VAR(emitter, tcpRecvPort);
	EMIT_VAR(emitter, tcpServPort);

	emitter->EndMap();
}

// controls ////////////////////////////////////////////////////////////////////

void Config::controlsHash_t::Load(yaml::MapNode* root)
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

void Config::controlsHash_t::Save(yaml::Emitter* emitter) const
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

void Config::cameraHash_t::Load(yaml::MapNode* root)
{
	if (root == NULL) return;

	READ_INT(root, zoomIn, 0, INT_MAX);
	READ_INT(root, zoomOut, 0, INT_MAX);
	READ_INT(root, panUp, 0, INT_MAX);
	READ_INT(root, panDown, 0, INT_MAX);
	READ_INT(root, reset, 0, INT_MAX);
}

void Config::cameraHash_t::Save(yaml::Emitter* emitter) const
{
	emitter->MapKey("camera_hash");
	emitter->StartMap();

	EMIT_VAR(emitter, zoomIn);
	EMIT_VAR(emitter, zoomOut);
	EMIT_VAR(emitter, panUp);
	EMIT_VAR(emitter, panDown);
	EMIT_VAR(emitter, reset);

	emitter->EndMap();
}

void Config::ui_t::Load(yaml::MapNode* root)
{
	if (root == NULL) return;

	READ_INT(root, consoleToggle, 0, INT_MAX);
	READ_INT(root, consoleUp, 0, INT_MAX);
	READ_INT(root, consoleDown, 0, INT_MAX);
	READ_INT(root, consoleTop, 0, INT_MAX);
	READ_INT(root, consoleBottom, 0, INT_MAX);
	READ_INT(root, consoleHelp, 0, INT_MAX);

	READ_INT(root, menuOk, 0, INT_MAX);
	READ_INT(root, menuCancel, 0, INT_MAX);
	READ_INT(root, menuExtra, 0, INT_MAX);
	READ_INT(root, menuUp, 0, INT_MAX);
	READ_INT(root, menuDown, 0, INT_MAX);
	READ_INT(root, menuLeft, 0, INT_MAX);
	READ_INT(root, menuRight, 0, INT_MAX);
	READ_INT(root, menuNext, 0, INT_MAX);
	READ_INT(root, menuPrev, 0, INT_MAX);
}

void Config::ui_t::Save(yaml::Emitter* emitter) const
{
	emitter->MapKey("ui");
	emitter->StartMap();

	EMIT_VAR(emitter, consoleToggle);
	EMIT_VAR(emitter, consoleUp);
	EMIT_VAR(emitter, consoleDown);
	EMIT_VAR(emitter, consoleTop);
	EMIT_VAR(emitter, consoleBottom);
	EMIT_VAR(emitter, consoleHelp);

	EMIT_VAR(emitter, menuOk);
	EMIT_VAR(emitter, menuCancel);
	EMIT_VAR(emitter, menuExtra);
	EMIT_VAR(emitter, menuUp);
	EMIT_VAR(emitter, menuDown);
	EMIT_VAR(emitter, menuLeft);
	EMIT_VAR(emitter, menuRight);
	EMIT_VAR(emitter, menuNext);
	EMIT_VAR(emitter, menuPrev);

	emitter->EndMap();
}

}  // namespace Util
}  // namespace HoverRace
