
/* StdAfx.h
	Precompiled header for client. */

#pragma once

#include "../include/util/os.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <exception>
#include <forward_list>
#include <functional>
#include <locale>
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

#include "../../include/compat/luabind_cxx11.h"
#include "../../include/compat/boost_cxx11.h"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/trivial.hpp>
#include <boost/optional.hpp>
#include <boost/signals2.hpp>
#include <boost/thread/thread.hpp>

#include "../../engine/Script/Lua.h"

#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#ifdef _WIN32
#	pragma warning(pop)
#endif

#define PACKAGE_NAME_WIDEN(x) L ## x
#define PACKAGE_NAME_APPLY(x) PACKAGE_NAME_WIDEN(x)
#define PACKAGE_NAME_L PACKAGE_NAME_APPLY(PACKAGE_NAME)

#include "../include/util/i18n.h"
#include "../include/util/util.h"

#define HR_WEBSITE "http://www.hoverrace.com/"
