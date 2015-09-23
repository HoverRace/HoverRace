
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
#	define pgettext(p, x) ::boost::locale::translate(p, x)

#else

#	define _(x) (x)
#	define gettext(x) (x)
#	define dgettext(d,x) (x)
#	define dcgettext(d,x,c) (x)
#	define pgettext(p,x) (x)
#	define pgettextImpl(f,x) (x)

#endif  // ENABLE_NLS
