
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

#include <boost/filesystem.hpp>
#include <utf8/utf8.h>

#include "yaml/Emitter.h"
#include "yaml/MapNode.h"

#include "Config.h"
#include "Log.h"
#include "Str.h"

#include "Locale.h"

namespace fs = boost::filesystem;

namespace HoverRace {
namespace Util {

namespace {

/**
 * Locale ID to full language name.
 * Note: These strings are intentionally *not* localized.
 */
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
	path(path), domain(domain)
{
	ScanLocales();
}

/**
 * Normalize the locale ID.
 * This removes extraneous whitespace and invalid characters.
 * @param s The original locale ID.
 * @return The normalized ID.
 */
std::string Locale::NormalizeId(const std::string &s)
{
	std::string retv;
	auto appender = std::back_inserter(retv);

	size_t n = 0;
	for (auto iter = s.cbegin(), iend = s.cend(); iter != iend;) {
		MR_UInt32 ch = utf8::next(iter, iend);

		if ((ch >= 'A' && ch <= 'Z') ||
			(ch >= 'a' && ch <= 'z') ||
			ch == '_' || ch == '@' || ch == '.')
		{
			utf8::append(ch, appender);

			n++;
			if (n >= 32) break;
		}
	}

	return retv;
}

/**
 * Attempt to change the global locale to the preferred locale.
 */
void Locale::RequestPreferredLocale()
{
	RequestLocale(Config::GetInstance()->i18n.preferredLocale);
}

/**
 * Attempt to change the global locale.
 * @param id The locale ID.
 */
void Locale::RequestLocale(const std::string &id)
{
#	ifndef ENABLE_NLS
		selectedLocaleId = "en_US";
#	else
		auto &locale = OS::SetLocale(path, domain, NormalizeId(id));

		// Try to determine which of our supported locales (if any) were chosen.
		selectedLocaleId = boost::none;
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

		HR_LOG(debug) << "Locale did not match known locale; using en_US.";
#	endif
}

/**
 * Find the locale name for the given locale ID.
 * @param id The locale ID.
 * @return The locale name (never empty).
 */
const std::string &Locale::IdToName(const std::string &id) const
{
	static const std::string UNKNOWN = "Unknown";

	auto iter = availableLocales.find(id);
	return iter == availableLocales.end() ? UNKNOWN : iter->second;
}

/**
 * Scan the locale directory for available locales.
 * This will invalidate all iterators.
 */
void Locale::ScanLocales()
{
	availableLocales.clear();
	availableLocales.insert(*(LOCALE_NAMES.find("en_US")));

#	ifdef ENABLE_NLS
		if (!fs::exists(path)) {
			HR_LOG(debug) <<
				"Locale path does not exist (skipping scan): " << path;
			return;
		}

		if (!fs::is_directory(path)) {
			HR_LOG(error) <<
				"Locale path is not a directory (skipping scan): " << path;
			return;
		}

		auto domainPath =
			Str::UP("LC_MESSAGES") /
			Str::UP(domain + ".mo");

		fs::directory_iterator iterEnd;
		for (fs::directory_iterator iter{path}; iter != iterEnd; ++iter) {
			auto lpath = iter->path();
			auto id = lpath.filename().string();
			HR_LOG(debug) << "Scanning locale dir: " << id;

			auto dpath = lpath / domainPath;
			if (fs::exists(dpath)) {
				HR_LOG(debug) << "Found locale " << id << " at: " << dpath;

				auto nameIter = LOCALE_NAMES.find(id);
				if (nameIter != LOCALE_NAMES.end()) {
					availableLocales.emplace(id, nameIter->second);
				}
				else {
					HR_LOG(warning) << "No name for locale: " << id;
					availableLocales.emplace(id, id);
				}
			}
		}
#	endif
}

Locale::const_iterator Locale::cbegin() const
{
	return availableLocales.cbegin();
}

Locale::const_iterator Locale::cend() const
{
	return availableLocales.cend();
}

}  // namespace Util
}  // namespace HoverRace
