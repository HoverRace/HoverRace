
// Log.cpp
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

#include "StdAfx.h"

#include <SDL2/SDL_log.h>

#include "Config.h"

#include "Log.h"

namespace HoverRace {
namespace Util {

namespace Log {

logAdded_t logAddedSignal;

namespace {
	SDL_LogOutputFunction sdlBuiltinLog = nullptr;

	void LogCallback(void *userData, int category, SDL_LogPriority priority,
                     const char *message)
	{
		Level::level_t pri;
		switch (priority) {
			case SDL_LOG_PRIORITY_DEBUG: pri = Level::DEBUG; break;
			case SDL_LOG_PRIORITY_INFO: pri = Level::INFO; break;
			case SDL_LOG_PRIORITY_WARN: pri = Level::WARN; break;
			case SDL_LOG_PRIORITY_ERROR: pri = Level::ERROR; break;
			case SDL_LOG_PRIORITY_CRITICAL: pri = Level::FATAL; break;
			default: return; // Every other log level is ignored.
		}

		// Broadcast the log message to all subscribers.
		Entry entry = { pri, message };
		logAddedSignal(entry);

		// Let SDL handle the platform-specific output.
		sdlBuiltinLog(userData, category, priority, message);
	}
}

/**
 * Initialize the system log.
 */
void Init()
{
	const bool verboseLog = Config::GetInstance()->runtime.verboseLog;
#	ifdef _DEBUG
		SDL_LogSetAllPriority(verboseLog ? SDL_LOG_PRIORITY_DEBUG : SDL_LOG_PRIORITY_INFO);
#	else
		SDL_LogSetAllPriority(verboseLog ? SDL_LOG_PRIORITY_INFO : SDL_LOG_PRIORITY_ERROR);
#	endif

	SDL_LogGetOutputFunction(&sdlBuiltinLog, nullptr);
	SDL_LogSetOutputFunction(LogCallback, nullptr);
}

void Debug(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, fmt, ap);
	va_end(ap);
}

void Info(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, fmt, ap);
	va_end(ap);
}

void Warn(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, fmt, ap);
	va_end(ap);
}

void Error(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, fmt, ap);
	va_end(ap);
}

void Fatal(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL, fmt, ap);
	va_end(ap);
}

};

}  // namespace Util
}  // namespace HoverRace
