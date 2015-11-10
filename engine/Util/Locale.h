
// Locale.h
//
// Copyright (c) 2015 Michael Imamura.
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

#include "OS.h"

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
		namespace yaml {
			class Emitter;
			class MapNode;
		}
	}
}

namespace HoverRace {
namespace Util {

/**
 * A library of available locales.
 * @author Michael Imamura
 */
class Locale
{
public:
	Locale(const OS::path_t &path, const std::string &domain);

public:
	const std::string &GetPreferredLocale() const { return preferredLocale; }
	void SetPreferredLocale(const std::string &id) { preferredLocale = id; }

	void RequestPreferredLocale();

	void RequestLocale(const std::string &id);

	/**
	 * Return the ID of the locale that was selected by RequestLocale().
	 * @return The locale ID, if set.
	 *         If not set, then the requested locale was unavailable, so
	 *         the default is being used instead.
	 */
	const boost::optional<std::string> &GetSelectedLocaleId() const { return selectedLocaleId; }

	const std::string &IdToName(const std::string &id) const;

private:
	using locales_t = std::map<std::string, std::string>;

	void ScanLocales();

public:
	void ResetToDefaults();
	void Load(yaml::MapNode *root);
	void Save(yaml::Emitter *emitter);

public:
	// STL-like iteration.
	using iterator = locales_t::iterator;
	using const_iterator = locales_t::const_iterator;
	using value_type = locales_t::value_type;
	const_iterator begin() const { return cbegin(); }
	const_iterator end() const { return cend(); }
	const_iterator cbegin() const;
	const_iterator cend() const;

private:
	OS::path_t path;
	std::string domain;
	std::string preferredLocale;
	boost::optional<std::string> selectedLocaleId;
	locales_t availableLocales;  ///< Cached list of available locales.
};

}  // namespace Util
}  // namespace HoverRace

#undef MR_DllDeclare
