
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

#include <boost/shared_ptr.hpp>

#ifdef _WIN32
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

#		ifdef _WIN32
			typedef DWORD timestamp_t;
#		else
			typedef unsigned long long timestamp_t;
#		endif
		
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
		static boost::shared_ptr<monitors_t> GetMonitors();

#		ifdef _WIN32
			static std::string GuidToString(const GUID &guid);
			static void StringToGuid(const std::string &s, GUID &guid);
#		endif

		static void TimeInit();
		static timestamp_t Time();
		static timestamp_t TimeDiff(timestamp_t a, timestamp_t b);
		static void TimeShutdown();

		static bool OpenLink(const std::string &url);

		static void Free(void *buf);
};

MR_DllDeclare inline bool operator==(const OS::Resolution &l, const OS::Resolution &r)
{
	return l.w == r.w && l.h == r.h;
}

MR_DllDeclare inline bool operator<(const OS::Resolution &l, const OS::Resolution &r)
{
	return l.w < r.w || !(r.w < l.w) && l.h < r.h;
}

}  // namespace Util
}  // namespace HoverRace

#undef MR_DllDeclare
