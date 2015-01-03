/* StdAfx.h
	Precompiled header for mazecompiler. */

#pragma once

#include "../../include/util/os.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED

#include <locale.h>
#include <math.h>
#include <stdio.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#ifdef _WIN32
#	pragma warning(push, 0)
#endif

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#ifdef _WIN32
#	pragma warning(pop)
#endif

#include "../../include/util/i18n.h"
#include "../../include/util/util.h"
