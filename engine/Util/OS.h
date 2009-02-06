
/* OS.h
	Header for OS.
*/

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

namespace HoverRace {
namespace Util {

class MR_DllDeclare OS {

	public:
		static void SetEnv(const char *key, const char *val);
		static void SetLocale();

		static void Free(void *buf);

};

}  // namespace Util
}  // namespace HoverRace
