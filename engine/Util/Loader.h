
// Loader.h
//
// Copyright (c) 2014, 2015 Michael Imamura.
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

#include <queue>

#include "Log.h"

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
 * Queue of resource loaders.
 * @author Michael Imamura
 */
class Loader
{
public:
	Loader() { }
	~Loader() { }

public:
	bool IsEmpty() const { return loaders.empty(); }

public:
	using finishedLoadingSignal_t = boost::signals2::signal<void()>;

	/**
	 * Fired when all resources have been loaded and the loading scene
	 * is shutting down.
	 * @return The finished loading signal.
	 */
	finishedLoadingSignal_t &GetFinishedLoadingSignal()
	{
		return finishedLoadingSignal;
	}

	void FireFinishedLoadingSignal()
	{
		finishedLoadingSignal();
	}

private:
	using loader_t = std::pair<std::string, std::function<void()>>;
public:
	/**
	 * Add a new named loader.
	 * The loader name is used for logging purposes only.
	 * @param s The name of the loader.
	 * @param fn The loader function.
	 */
	template<class Fn>
	void AddLoader(const std::string &s, Fn fn)
	{
		loaders.emplace(s, fn);
	}

	/**
	 * Add a new unnamed loader.
	 * @param fn The loader function.
	 */
	template<class Fn>
	void AddLoader(Fn fn)
	{
		std::ostringstream oss;
		oss << "Loader " << loaders.size();
		loaders.emplace(oss.str(), fn);
	}

public:
	/**
	 * Load the next item.
	 *
	 * If all of the loaders have been executed, then the
	 * finishedLoadingSignal will *not* be fired automatically, since the
	 * owner of the loader may want to perform some actions before
	 * notifying the listeners.
	 *
	 * @return @c true if there are any loaders remaining,
	 *         @c false if all loaders have executed.
	 */
	bool LoadNext()
	{
		if (loaders.empty()) {
			return false;
		}

		auto &loader = loaders.front();
		HR_LOG(info) << "Loading: " << loader.first;
		loader.second();
		loaders.pop();

		return !loaders.empty();
	}

private:
	std::queue<loader_t> loaders;
	finishedLoadingSignal_t finishedLoadingSignal;
};

}  // namespace Util
}  // namespace HoverRace

#undef MR_DllDeclare
