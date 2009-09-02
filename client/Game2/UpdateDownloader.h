
// UpdateDownloader.h
// A class that can be used to download and apply updates for HoverRace.
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

#ifndef UPDATE_DOWNLOADER_H
#define UPDATE_DOWNLOADER_H

#include "../../engine/Net/CancelFlag.h"

#include <string.h>

using namespace std;

namespace HoverRace {
namespace Client {

class UpdateDownloader
{
	public:
		UpdateDownloader();
		~UpdateDownloader();

		bool CheckUrl(const std::string &url,
			Net::CancelFlagPtr cancelFlag=Net::CancelFlagPtr());

		string updateUrl;

	private:
		class Version {
			public:
				Version();
				Version(string ver);
				Version(long major, long minor, long patch, long rev);

				string toString();

				long major;
				long minor;
				long patch;
				long rev;

				bool operator>(Version &);
				bool operator<(Version &);
				bool operator==(Version &);
		};

	public:
		Version currentVersion;
		Version updatedVersion;
};
typedef boost::shared_ptr<UpdateDownloader> UpdateDownloaderPtr;

}  // namespace Client
}  // namespace HoverRace

#endif