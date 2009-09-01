// UpdateDownloader.cpp
// Downloads and applies updates from the central update server.
//
// Copyright (c) 2009 Ryan Curtin
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

#include "../../engine/Util/Config.h"

#include "../../engine/Net/Agent.h"
#include "../../engine/Net/NetExn.h"
#include "../../engine/Util/OS.h"

#include "UpdateDownloader.h"

using namespace std;
using namespace HoverRace;
using namespace HoverRace::Client;
using namespace HoverRace::Util;

UpdateDownloader::UpdateDownloader()
{
	// nothing to do
}

UpdateDownloader::~UpdateDownloader()
{
	// nothing to do
}

/***
 * Check the specified update server to see if HoverRace can be updated.
 * If we are a development build (cfg->IsPrerelease()) we will only attempt to update
 * to the newest development version, and stable versions (!cfg->IsPrerelease()) will
 * only attempt to update to the newest stable version.
 *
 * @param url Address of update server (stored in config)
 * @param cancelFlag Pointer to transfer cancel flag
 *
 * @return
 *   true if an update is required
 *   false if the current version is up to date
 *
 * @throws NetExn if transfer fails for some reason
 */
bool UpdateDownloader::CheckUrl(const std::string &url, Net::CancelFlagPtr cancelFlag)
{
	Config *cfg = Config::GetInstance();

	Net::Agent agent(url);
	//TODO: Set max size.

	stringstream io;
	agent.Get(io, cancelFlag);

	string in;

	getline(io, in);
	if(in != "VERSION LIST")
		throw Net::NetExn("Invalid format: Missing preamble");

	io.exceptions(std::istream::badbit | std::istream::failbit);

	string stableVersion;
	string develVersion;

	string serverLocation;

	for( ; ; ) {
		string type;
		try {
			io >> type;
		} catch (istream::failure &) {
			break; // EOF
		}

		if(type == "stable") {
			io >> stableVersion;
		} else if(type == "devel") {
			io >> develVersion;
		} else if(type == "server") {
			io >> serverLocation;
		}
	}

	// verify information
	if(stableVersion == "")
		throw Net::NetExn("Cannot find newest stable version");
	if(develVersion == "" && cfg->IsPrerelease())
		throw Net::NetExn("Cannot find newest development version");
	if(serverLocation == "")
		throw Net::NetExn("Cannot find update server");

	currentVersion = Version(cfg->GetFullVersion());
	updatedVersion = Version((cfg->IsPrerelease()) ? develVersion : stableVersion);

	if(updatedVersion > currentVersion) {
		// update is necessary
		// generate url
		char buffer[500];
		sprintf(buffer, "hoverrace-update-%ld.%ld.%ld.%ld-to-%ld.%ld.%ld.%ld.zip",
			currentVersion.major, currentVersion.minor, currentVersion.patch, currentVersion.rev,
			updatedVersion.major, updatedVersion.minor, updatedVersion.patch, updatedVersion.rev);

		updateUrl = buffer;
		return true;
	}

	return false;
}

/***
 * Download the necessary update to the given directory.
 *
 * @param dir Directory to download patchfile to
 *
 * @return
 *	 true if download is successful
 *   false if download fails
 */
bool UpdateDownloader::DownloadUpdate(string dir)
{
	// nothing yet
	return true;
}

/***
 * Apply the downloaded update, using the updater.exe utility.
 *
 * HoverRace will exit while that program is running!  Once patched
 * it will then be restarted.
 */
void UpdateDownloader::ApplyUpdate()
{
	MessageBox(NULL, "We would run updater.exe", "info", MB_OK);
}

/***
 * Default Version constructor.  0.0.0.0 default value.
 */
UpdateDownloader::Version::Version()
{
	major = minor = patch = rev = 0;
}

/***
 * Create a new Version object, using the input string which is delimited by
 * periods.  i.e. "1.2.3.4"
 *
 * @param ver String version, delimited by periods
 */
UpdateDownloader::Version::Version(string ver)
{
	sscanf(ver.c_str(), "%ld.%ld.%ld.%ld", &major, &minor, &patch, &rev);
}

/***
 * Create a new Version object.
 */
UpdateDownloader::Version::Version(long major, long minor, long patch, long rev)
{
	this->major = major;
	this->minor = minor;
	this->patch = patch;
	this->rev   = rev;
}

string UpdateDownloader::Version::toString()
{
	char buffer[500];
	sprintf(buffer, "%ld.%ld.%ld.%ld",
		major, minor, patch, rev);
	string ret = buffer;
	return ret;
}

bool UpdateDownloader::Version::operator<(UpdateDownloader::Version &x)
{
	if(major < x.major)
		return true;
	else if(major == x.major) {
		if(minor < x.minor)
			return true;
		else if(minor == x.minor) {
			if(patch < x.patch)
				return true;
			else if(patch == x.patch) {
				if(rev < x.rev)
					return true;
			}
		}
	}
	return false;
}

bool UpdateDownloader::Version::operator>(UpdateDownloader::Version &x)
{
	if(major > x.major)
		return true;
	else if(major == x.major) {
		if(minor > x.minor)
			return true;
		else if(minor == x.minor) {
			if(patch > x.patch)
				return true;
			else if(patch == x.patch) {
				if(rev > x.rev)
					return true;
			}
		}
	}
	return false;
}

bool UpdateDownloader::Version::operator==(UpdateDownloader::Version &x)
{
	return ((major == x.major) && (minor == x.minor) && (patch == x.patch) && (rev == x.rev));
}