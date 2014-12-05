
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

/// Alias for BOOST_LOG_TRIVIAL.
#define HR_LOG(lvl) BOOST_LOG_TRIVIAL(lvl)

namespace HoverRace {
namespace Util {

namespace Log {

	namespace detail {
		const size_t MAX_LOG = 512; ///< The maximum length of a vararg log.

		MR_DllDeclare std::string Fmt(const char *fmt, va_list ap);
	}

	MR_DllDeclare void Init();

	enum class Level {
		// Intentionally aligned with boost::log::trivial to avoid conversions.
		TRACE = boost::log::trivial::trace,
		DEBUG = boost::log::trivial::debug,
		INFO = boost::log::trivial::info,
		WARN = boost::log::trivial::warning,
		ERROR = boost::log::trivial::error,
		FATAL = boost::log::trivial::fatal,
	};

	struct Entry {
		Entry(const Level level, const char *message) :
			level(level), message(message) { }
		Entry &operator=(const Entry&) = delete;

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

	inline void Info(const char *fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		BOOST_LOG_TRIVIAL(info) << detail::Fmt(fmt, ap);
		va_end(ap);
	}

	inline void Warn(const char *fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		BOOST_LOG_TRIVIAL(warning) << detail::Fmt(fmt, ap);
		va_end(ap);
	}

	inline void Error(const char *fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		BOOST_LOG_TRIVIAL(error) << detail::Fmt(fmt, ap);
		va_end(ap);
	}

	inline void Fatal(const char *fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		BOOST_LOG_TRIVIAL(fatal) << detail::Fmt(fmt, ap);
		va_end(ap);
	}

}

}  // namespace Util
}  // namespace HoverRace

#undef MR_DllDeclare
