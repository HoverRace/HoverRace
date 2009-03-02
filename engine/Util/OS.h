
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

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

#include <set>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

namespace HoverRace {
namespace Util {

/**
 * Operating system support utilities.
 * @author Michael Imamura
 */
class MR_DllDeclare OS {

	public:
		static void SetEnv(const char *key, const char *val);
		static void SetLocale();
		
		struct Resolution
		{
			int w;
			int h;
			Resolution(int w, int h) : w(w), h(h) { }
		};
		typedef std::set<Resolution> resolutions_t;
		struct Monitor
		{
			bool primary;
			std::string name;
			resolutions_t resolutions;
		};
		typedef std::vector<Monitor> monitors_t;
		static boost::shared_ptr<monitors_t> GetMonitors();

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
