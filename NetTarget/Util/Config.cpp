
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

#include "Config.h"

#ifdef _WIN32
	#define DIRSEP "\\"
#else
	#define DIRSEP "/"
#endif
#define CONFIG_FILENAME "config.yml"

#define EMIT_VAR(emitter,name) \
	(emitter)->MapKey(#name); \
	(emitter)->Value(name);

MR_Config *MR_Config::instance = NULL;

/**
 * Create a new instance using the specified directory.
 * @param path (Optional) The directory for storing configuration data.
 *             The default is to use {@link #GetDefaultPath()}.
 */
MR_Config::MR_Config(const std::string &path)
{
	this->path = (path.length() == 0) ? GetDefaultPath() : path;

	// Set initial defaults.
	ResetToDefaults();

	// Load and parse the YAML file.
	Load();
}

MR_Config::~MR_Config()
{
}


/**
 * Initialize the singleton instance.
 * @param path (Optional) The directory for storing configuration data.
 *             The default is to use {@link #GetDefaultPath()}.
 */
void MR_Config::Init(const std::string &path)
{
	if (instance == NULL) {
		instance = new MR_Config(path);
	}
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
 * Resets the configuration to the default "factory" settings.
 */
void MR_Config::ResetToDefaults()
{
	/*
	graphics.brightness = 1.2;
	graphics.contrast = 0.95;
	graphics.gamma = 0.95;
	graphics.nativeBppFullscreen = false;
	*/

	misc.displayFirstScreen = true;
	misc.introMovie = true;
}

/**
 * Load the configuration.
 * Does nothing if the configuration file doesn't exist.
 * @throws std::exception If an error occurs while reading.
 */
void MR_Config::Load()
{
	const std::string &retv = GetConfigFilename();
	//TODO
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
		misc.Save(emitter);

		emitter->EndMap();
		delete emitter;
	} catch (yaml::EmitterExn &ex) {
		if (emitter != NULL) delete emitter;
		fclose(out);
		throw std::exception(ex);
	}

	fclose(out);
}

void MR_Config::SaveVersion(yaml::Emitter *emitter)
{
	emitter->MapKey("version");
	//TODO: Use real HoverRace version.
	emitter->Value("1.0");
}

void MR_Config::cfg_misc_t::Save(yaml::Emitter *emitter)
{
	emitter->MapKey("misc");
	emitter->StartMap();

	EMIT_VAR(emitter, displayFirstScreen);
	EMIT_VAR(emitter, introMovie);

	emitter->EndMap();
}
