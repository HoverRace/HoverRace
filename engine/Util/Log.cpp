
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
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#ifdef _WIN32
#	include <boost/log/expressions/predicates/is_debugger_present.hpp>
#	include <boost/log/sinks/debug_output_backend.hpp>
#endif
#include <boost/make_shared.hpp>

#include <SDL2/SDL_log.h>

#include "Config.h"

#include "Log.h"

namespace HoverRace {
namespace Util {

namespace Log {

logAdded_t logAddedSignal;

namespace {

/**
 * Query if there is anybody listening to log events.
 * @return @c true if there are listeners, @c false otherwise.
 */
bool HasLogAddedSlots()
{
	return !logAddedSignal.empty();
}

/**
 * Custom backend that fires log events.
 */
class LogSignalSinkBackend :
	public boost::log::sinks::basic_formatted_sink_backend<
		char,
		boost::log::sinks::synchronized_feeding>
{
	public:
		void consume(const boost::log::record_view &rec, const string_type &s)
		{
			using namespace boost::log;

			Level pri = Level::INFO;
			visit(trivial::severity, rec, [&pri](trivial::severity_level sev) {
				switch (sev) {
					case trivial::debug: pri = Level::DEBUG; break;
					case trivial::info: pri = Level::INFO; break;
					case trivial::warning: pri = Level::WARN; break;
					case trivial::error: pri = Level::ERROR; break;
					case trivial::fatal: pri = Level::FATAL; break;
					default: return; // Every other log level is ignored.
				}
			});

			// Broadcast the log message to all subscribers.
			Entry entry = { pri, s.c_str() };
			logAddedSignal(entry);
		}
};

void LogCallback(void*, int, SDL_LogPriority priority, const char *message)
{
	switch (priority) {
		case SDL_LOG_PRIORITY_DEBUG:
			BOOST_LOG_TRIVIAL(debug) << message;
			break;
		case SDL_LOG_PRIORITY_INFO:
			BOOST_LOG_TRIVIAL(info) << message;
			break;
		case SDL_LOG_PRIORITY_WARN:
			BOOST_LOG_TRIVIAL(warning) << message;
			break;
		case SDL_LOG_PRIORITY_ERROR:
			BOOST_LOG_TRIVIAL(error) << message;
			break;
		case SDL_LOG_PRIORITY_CRITICAL:
			BOOST_LOG_TRIVIAL(fatal) << message;
			break;
		default: return; // Every other log level is ignored.
	}
}

/**
 * Add a streaming sink to the logger.
 */
void AddStreamLog()
{
	using namespace boost::log;
	namespace expr = boost::log::expressions;

	typedef sinks::text_ostream_backend backend_t;
	auto backend = boost::make_shared<backend_t>();
	backend->add_stream(boost::shared_ptr<std::ostream>(
		&std::clog, [](const void*){}));
	backend->auto_flush(true);

	typedef sinks::synchronous_sink<backend_t> sink_t;
	auto sink = boost::make_shared<sink_t>(backend);
	sink->set_formatter(expr::stream << '[' << trivial::severity << "] " <<
		expr::message);
	core::get()->add_sink(sink);
}

/**
 * Add a sink to notify event listeners.
 */
void AddLogSignalLog()
{
	using namespace boost::log;
	namespace expr = boost::log::expressions;

	typedef LogSignalSinkBackend backend_t;
	auto backend = boost::make_shared<backend_t>();

	typedef sinks::synchronous_sink<backend_t> sink_t;
	auto sink = boost::make_shared<sink_t>(backend);
	// Only fire the backend if there are any actual listeners.
	// Using std::bind here to throw away the parameters since we don't use
	// them.
	sink->set_filter(std::bind(HasLogAddedSlots));
	sink->set_formatter(expr::stream << '[' << trivial::severity << "] " <<
		expr::message);
	core::get()->add_sink(sink);
}

#ifdef _WIN32
/**
 * Enable logging to the Windows debugger output window.
 */
void AddWindowsDebugLog()
{
	using namespace boost::log;
	namespace expr = boost::log::expressions;

	// Make sure we log using the wchar_t version.
	typedef sinks::basic_debug_output_backend<wchar_t> backend_t;
	typedef sinks::synchronous_sink<backend_t> sink_t;
	auto sink = boost::make_shared<sink_t>();
	sink->set_filter(expr::is_debugger_present());
	sink->set_formatter(expr::stream << L'[' << trivial::severity << L"] " <<
		expr::message << L'\n');
	core::get()->add_sink(sink);
}
#endif

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

	const bool verboseLog = Config::GetInstance()->runtime.verboseLog;
	auto core = core::get();

	AddStreamLog();
	AddLogSignalLog();
#	ifdef _WIN32
		AddWindowsDebugLog();
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
