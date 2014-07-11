
// OS.h
// Header for operating system support utilities.
//
// Copyright (c) 2009 Michael Imamura.
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

#include <locale>
#include <set>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include "MR_Types.h"

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
namespace Util {

/**
 * Operating system support utilities.
 * @author Michael Imamura
 */
class MR_DllDeclare OS {

	public:
		static int nibbles[256];
		static std::locale locale;
		static const std::locale stdLocale;

		typedef MR_UInt32 timestamp_t;

#		if defined(WITH_WIDE_PATHS) && BOOST_FILESYSTEM_VERSION == 2
			typedef boost::filesystem::wpath path_t;
			typedef boost::filesystem::wdirectory_iterator dirIter_t;
			typedef boost::filesystem::wdirectory_entry dirEnt_t;
#		else
			typedef boost::filesystem::path path_t;
			typedef boost::filesystem::directory_iterator dirIter_t;
			typedef boost::filesystem::directory_entry dirEnt_t;
#		endif

#		if BOOST_FILESYSTEM_VERSION == 2
			typedef boost::filesystem::basic_filesystem_error<path_t> fs_error_t;
#		else
			typedef boost::filesystem::filesystem_error fs_error_t;
#		endif
		typedef path_t::value_type *pstr_t;
		typedef const path_t::value_type *cpstr_t;

		static void SetEnv(const char *key, const char *val);
		static void SetLocale();

		struct MR_DllDeclare Resolution
		{
			int w;
			int h;
			Resolution(int w, int h) : w(w), h(h) { }
			Resolution(const std::string &s);
			std::string AsString() const;
		};
		typedef std::set<Resolution> resolutions_t;
		struct MR_DllDeclare Monitor
		{
			bool primary;
			std::string id;
			std::string name;
			resolutions_t resolutions;
		};
		typedef std::vector<Monitor> monitors_t;
		static std::shared_ptr<monitors_t> GetMonitors();

#		ifdef _WIN32
			static std::string GuidToString(const GUID &guid);
			static void StringToGuid(const std::string &s, GUID &guid);
#		endif

#		ifndef _WIN32
			static std::string StrError(int errnum);
#		endif

		static void TimeInit();
		static timestamp_t Time();
		/**
		 * Calculate the difference between two timestamps.
		 * This properly handles wraparound in timestamps.
		 * @param laterTs The later timestamp.
		 * @param earlierTs The earlier timestamp.
		 * @return @p laterTs - @p earlierTs
		 */
		static timestamp_t TimeDiff(timestamp_t laterTs, timestamp_t earlierTs)
		{
			if (laterTs >= earlierTs) {
				return laterTs - earlierTs;
			}
			else {
				return 0xfffffffful - (earlierTs - laterTs) + 1;
			}
		}
		static void TimeShutdown();

		static bool OpenLink(const std::string &url);
		static bool OpenPath(const path_t &path);

		static FILE *FOpen(const path_t &path, const char *mode);

		static void Free(void *buf);
};

MR_DllDeclare inline bool operator==(const OS::Resolution &l, const OS::Resolution &r)
{
	return l.w == r.w && l.h == r.h;
}

MR_DllDeclare inline bool operator<(const OS::Resolution &l, const OS::Resolution &r)
{
	return l.w < r.w || (!(r.w < l.w) && l.h < r.h);
}

}  // namespace Util
}  // namespace HoverRace

#undef MR_DllDeclare
