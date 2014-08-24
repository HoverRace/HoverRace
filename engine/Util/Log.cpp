
// Log.cpp
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

#include "../StdAfx.h"

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#ifdef _WIN32
#	include <boost/log/expressions/predicates/is_debugger_present.hpp>
#	include <boost/log/sinks/sync_frontend.hpp>
#	include <boost/log/sinks/debug_output_backend.hpp>
#endif

#include <SDL2/SDL_log.h>

#include "Config.h"

#include "Log.h"

namespace HoverRace {
namespace Util {

namespace Log {

logAdded_t logAddedSignal;

namespace {

void LogCallback(void *userData, int category, SDL_LogPriority priority,
                    const char *message)
{
	Level pri;
	switch (priority) {
		case SDL_LOG_PRIORITY_DEBUG:
			pri = Level::DEBUG;
			BOOST_LOG_TRIVIAL(debug) << message;
			break;
		case SDL_LOG_PRIORITY_INFO:
			pri = Level::INFO;
			BOOST_LOG_TRIVIAL(info) << message;
			break;
		case SDL_LOG_PRIORITY_WARN:
			pri = Level::DEBUG;
			BOOST_LOG_TRIVIAL(warning) << message;
			break;
		case SDL_LOG_PRIORITY_ERROR:
			pri = Level::DEBUG;
			BOOST_LOG_TRIVIAL(error) << message;
			break;
		case SDL_LOG_PRIORITY_CRITICAL:
			pri = Level::FATAL;
			BOOST_LOG_TRIVIAL(fatal) << message;
			break;
		default: return; // Every other log level is ignored.
	}

	// Broadcast the log message to all subscribers.
	Entry entry = { pri, message };
	logAddedSignal(entry);
}

}  // namespace

namespace detail {

std::string Fmt(const char *fmt, va_list ap)
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

}  // namespace detail


/**
 * Initialize the system log.
 */
void Init()
{
	using namespace boost::log;
	namespace expr = boost::log::expressions;

	const bool verboseLog = Config::GetInstance()->runtime.verboseLog;
	auto core = core::get();

#	ifdef _WIN32
		{
			// Enable logging to the debugger output window.
			typedef sinks::synchronous_sink<sinks::basic_debug_output_backend<wchar_t>> sink_t;
			auto sink = boost::make_shared<sink_t>();
			sink->set_filter(expr::is_debugger_present());
			sink->set_formatter(expr::stream << expr::message << L'\n');
			core->add_sink(sink);
		}
#	endif

#	ifdef _DEBUG
		if (verboseLog) {
			SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
			core->set_filter(
				trivial::severity >= trivial::debug
			);
		}
		else {
			SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);
			core->set_filter(
				trivial::severity >= trivial::info
			);
		}
#	else
		if (verboseLog) {
			SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);
			core->set_filter(
				trivial::severity >= trivial::info
			);
		}
		else {
			SDL_LogSetAllPriority(SDL_LOG_PRIORITY_ERROR);
			core->set_filter(
				trivial::severity >= trivial::error
			);
		}
#	endif

	SDL_LogSetOutputFunction(LogCallback, nullptr);
}

};

}  // namespace Util
}  // namespace HoverRace
