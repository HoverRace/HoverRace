
/* StdAfx.h
	Precompiled header for ParcelDump. */

#pragma once

#include "../../include/util/os.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED

#include <math.h>
#include <stdio.h>

#include <exception>
#include <iostream>
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

#include "../../include/util/util.h"
