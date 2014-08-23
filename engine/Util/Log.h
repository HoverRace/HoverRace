
// Log.h
//
// Copyright (c) 2013, 2014 Michael Imamura.
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

#include <boost/log/trivial.hpp>

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

// Previously defined in WinGDI.h.
#ifdef ERROR
#	undef ERROR
#endif

namespace HoverRace {
namespace Util {

namespace Log {

	namespace detail {
		const size_t MAX_LOG = 512;

		inline std::string Fmt(const char *fmt, va_list ap)
		{
			char buf[MAX_LOG];
			memset(buf, 0, sizeof(buf));
			if (vsnprintf(buf, sizeof(buf) - 1, fmt, ap) == -1) {
				std::string retv{buf};
				retv += "...<output truncated>";
				return retv;
			}
			else {
				return std::string(buf);
			}
		}
	}

	MR_DllDeclare void Init();

	enum class Level {
		DEBUG,
		INFO,
		WARN,
		ERROR,
		FATAL,
	};

	struct Entry {
		const Level level;
		const char *message;
	};

	typedef boost::signals2::signal<void(const Entry&)> logAdded_t;
	MR_DllDeclare extern logAdded_t logAddedSignal;

	inline void Debug(const char *fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		BOOST_LOG_TRIVIAL(debug) << detail::Fmt(fmt, ap);
		va_end(ap);
	}

#if 0
#ifdef _DEBUG
	MR_DllDeclare void Debug(const char *fmt, ...);
#else
	// Not a great solution (params are still evaluated), but suffices for now.
	MR_DllDeclare inline void Debug(...) { }
#endif
#endif
	MR_DllDeclare void Info(const char *fmt, ...);
	MR_DllDeclare void Warn(const char *fmt, ...);
	MR_DllDeclare void Error(const char *fmt, ...);
	MR_DllDeclare void Fatal(const char *fmt, ...);

}

}  // namespace Util
}  // namespace HoverRace

#undef MR_DllDeclare
