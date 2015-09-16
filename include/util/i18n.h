
/* Common defines for i18n support. */

#pragma once

#ifdef _
#	undef _
#endif
#ifdef ENABLE_NLS
#	ifdef _WIN32
#		pragma warning(push, 0)
#	endif

#	include <boost/locale.hpp>

#	ifdef _WIN32
#		pragma warning(pop)
#	endif

#	define _(x) ::boost::locale::translate(x)

	// Our own little version of pgettext() so we don't need all of gettext.h.
#	define pgettext(p,x) pgettextImpl(p "\004" x, x)
	static inline std::string pgettextImpl(const char *full, const char *msg)
	{
		auto retv = _(full).str();
		return (retv == full) ? std::string(msg) : retv;
	}

#else

#	define _(x) (x)
#	define gettext(x) (x)
#	define dgettext(d,x) (x)
#	define dcgettext(d,x,c) (x)
#	define pgettext(p,x) (x)
#	define pgettextImpl(f,x) (x)

#endif  // ENABLE_NLS
