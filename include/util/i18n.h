
/* Common defines for i18n support. */

#pragma once

#ifdef _
#	undef _
#endif
#ifdef ENABLE_NLS
#	include <libintl.h>

	// The gettext custom versions of printf (and the like) sometimes cause
	// crashes on Win32.  We use Boost Format for translated strings anyway,
	// so we don't need these.
#	ifdef _WIN32
#		undef printf
#		undef vprintf
#		undef fprintf
#		undef vfprintf
#		undef snprintf
#		undef vsnprintf
#	endif

#	define _(x) gettext(x)

	// Our own little version of pgettext() so we don't need all of gettext.h.
#	define pgettext(p,x) pgettextImpl(p "\004" x, x)
	static inline const char *pgettextImpl(const char *full, const char *msg)
	{
		const char *retv = _(full);
		return (retv == full) ? msg : retv;
	}

#else

#	define _(x) (x)
#	define gettext(x) (x)
#	define dgettext(d,x) (x)
#	define dcgettext(d,x,c) (x)
#	define pgettext(p,x) (x)
#	define pgettextImpl(f,x) (x)

#endif  // ENABLE_NLS
