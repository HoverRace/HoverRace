
/* Common defines for i18n support. */

#pragma once

#ifdef _WIN32
#	pragma warning(push, 0)
#endif

#include <boost/locale.hpp>

#ifdef _WIN32
#	pragma warning(pop)
#endif

#ifdef _
#	undef _
#endif
#define _(...) ::boost::locale::translate(__VA_ARGS__)
