
// Log.h
//
// Copyright (c) 2013 Michael Imamura.
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

// Previously defined in WinGDI.h.
#ifdef ERROR
#	undef ERROR
#endif

namespace HoverRace {
namespace Util {

namespace Log {

	MR_DllDeclare void Init();

	namespace Level {
		enum level_t {
			DEBUG,
			INFO,
			WARN,
			ERROR,
			FATAL,
		};
	}

	struct Entry {
		const Level::level_t level;
		const char *message;
	};

	typedef boost::signals2::signal<void(const Entry&)> logAdded_t;
	MR_DllDeclare extern logAdded_t logAddedSignal;

#ifdef _DEBUG
	MR_DllDeclare void Debug(const char *fmt, ...);
#else
	// Not a great solution (params are still evaluated), but suffices for now.
	MR_DllDeclare inline void Debug(...) { }
#endif
	MR_DllDeclare void Info(const char *fmt, ...);
	MR_DllDeclare void Warn(const char *fmt, ...);
	MR_DllDeclare void Error(const char *fmt, ...);
	MR_DllDeclare void Fatal(const char *fmt, ...);

}

}  // namespace Util
}  // namespace HoverRace

#undef MR_DllDeclare
