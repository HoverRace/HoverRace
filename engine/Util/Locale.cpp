
// Locale.cpp
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

#include "Log.h"

#include "Locale.h"

namespace HoverRace {
namespace Util {

namespace {

const std::map<std::string, std::string> LOCALE_NAMES = {
	{ "en_US", "English (United States)" },
#ifdef ENABLE_NLS
	{ "de", "German" },
	{ "en_AU", "English (Australia)" },
	{ "en_GB", "English (Great Britain)" },
	{ "en_NZ", "English (New Zealand)" },
	{ "en_US@piglatin", "Pig Latin" },
	{ "es", "Spanish" },
	{ "et", "Estonian" },
	{ "fi", "Finnish" },
	{ "fr", "French" },
	{ "ja", "Japanese" },
	{ "ko", "Korean" },
	{ "nb", "Norwegian" },
	{ "nl", "Dutch" },
	{ "pt", "Portuguese" },
	{ "pt_BR", "Portuguese (Brazil)" },
	{ "ro", "Romanian" },
	{ "sr", "Serbian" },
	{ "tlh", "Klingon" },
#endif
};

}  // namespace

Locale::Locale(const OS::path_t &path, const std::string &domain) :
	path(path), domain(domain), selectedLocaleId("en_US")
{
	//TODO: Scan locale path for available locales.
}

/**
 * Attempt to change the global locale.
 * @param id The locale ID.
 */
void Locale::RequestLocale(const std::string &id)
{
#	ifdef ENABLE_NLS
		auto &locale = OS::SetLocale(path, domain, id);

		// Try to determine which of our supported locales (if any) were chosen.
		const auto &facet = std::use_facet<boost::locale::info>(locale);
		const auto language = facet.language();
		const auto country = facet.country();
		const auto variant = facet.variant();

		const auto check = [&](const std::string &id) -> bool {
			auto iter = LOCALE_NAMES.find(id);
			if (iter != LOCALE_NAMES.end()) {
				HR_LOG(debug) << "Matched known locale: " << iter->first <<
					": " << iter->second;
				selectedLocaleId = iter->first;
				return true;
			}
			else {
				return false;
			}
		};

		if (!variant.empty()) {
			// We assume that if a variant was specified, then the country
			// was as well.
			if (check(language + "_" + country + "@" + variant)) return;
		}

		if (!country.empty()) {
			if (check(language + "_" + country)) return;
		}

		if (check(language)) return;

		selectedLocaleId = "en_US";
#	endif
}

Locale::const_iterator Locale::cbegin() const
{
	//TODO: Use scanned list.
	return LOCALE_NAMES.cbegin();
}

Locale::const_iterator Locale::cend() const
{
	return LOCALE_NAMES.cend();
}

}  // namespace Util
}  // namespace HoverRace
