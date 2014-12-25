
// OS.h
//
// Copyright (c) 2009, 2014 Michael Imamura.
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
struct MR_DllDeclare OS
{
	static std::locale locale;
	static const std::locale stdLocale;

	using timestamp_t = MR_Int64;

	using path_t = boost::filesystem::path;
	using dirIter_t = boost::filesystem::directory_iterator;
	using dirEnt_t =  boost::filesystem::directory_entry;

	using fs_error_t = boost::filesystem::filesystem_error;

	using pstr_t = path_t::value_type*;
	using cpstr_t = const path_t::value_type*;

	static void SetEnv(const char *key, const char *val);
	static void SetLocale();

#	ifndef _WIN32
		static std::string StrError(int errnum);
#	endif

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
		// We assume that signed 64-bit values won't have to deal with
		// wraparound.
		return laterTs - earlierTs;
	}
	static std::string FileTimeString();
	static void TimeShutdown();

	static bool OpenLink(const std::string &url);
	static bool OpenPath(const path_t &path);

	static FILE *FOpen(const path_t &path, const char *mode);
};

}  // namespace Util
}  // namespace HoverRace

#undef MR_DllDeclare
