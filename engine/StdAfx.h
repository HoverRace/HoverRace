
/* StdAfx.h
	Precompiled header for engine. */

#pragma once

#include "../include/util/os.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Commonly-used STL headers.
#include <algorithm>
#include <array>
#include <chrono>
#include <exception>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifdef _WIN32
#	pragma warning(push, 0)
#endif

#define BOOST_FILESYSTEM_NO_DEPRECATED

#include "../include/compat/luabind_cxx11.h"
#include "../include/compat/boost_cxx11.h"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/trivial.hpp>
#include <boost/optional.hpp>
#include <boost/signals2.hpp>

#include "../../engine/Script/Lua.h"

#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#ifdef _WIN32
#	pragma warning(pop)
#endif

#include "../include/util/i18n.h"
#include "../include/util/util.h"
