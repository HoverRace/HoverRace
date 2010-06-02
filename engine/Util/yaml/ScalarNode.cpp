
// yaml/ScalarNode.cpp
// Wrapper for LibYAML scalar nodes.
//
// Copyright (c) 2008, 2009 Michael Imamura.
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

#include "StdAfx.h"

#include <sstream>

#include "../Str.h"

#include "ScalarNode.h"

namespace Str = HoverRace::Util::Str;
using HoverRace::Util::OS;
using namespace yaml;

#ifndef _WIN32
#	include <strings.h>
#	define _stricmp strcasecmp
#endif

namespace {
	// Always use the "C" locale for reading numbers.
	const std::locale stdLocale("C");
}

/**
 * Constructor.
 * @param doc The containing document.
 * @param node The underlying LibYAML node (may not be NULL).
 */
ScalarNode::ScalarNode(yaml_document_t *doc, yaml_node_t *node) :
	SUPER(doc, node)
{
	value = (char*)node->data.scalar.value;
}

/// Destructor.
ScalarNode::~ScalarNode()
{
}

/**
 * Returns the string value.
 * @return The value.
 */
std::string ScalarNode::AsString() const
{
	return value;
}

/**
 * Convert this scalar to a boolean value.
 * @param def The default value if the value cannot be converted to a boolean.
 * @return The converted value.
 */
bool ScalarNode::AsBool(bool def) const
{
	// See spec: http://yaml.org/type/bool.html
	if (_stricmp(value, "y") == 0 ||
		_stricmp(value, "yes") == 0 ||
		_stricmp(value, "true") == 0 ||
		_stricmp(value, "on") == 0)
	{
		return true;
	}
	if (_stricmp(value, "n") == 0 ||
		_stricmp(value, "no") == 0 ||
		_stricmp(value, "false") == 0 ||
		_stricmp(value, "off") == 0)
	{
		return false;
	}
	return def;
}

/**
 * Convert this scalar to a double floating-point value.
 * @param def The default value if the value cannot be converted to a double.
 * @param min Minimum clamp value.
 * @param max Maximum clamp value.
 * @return The converted value.
 */
double ScalarNode::AsDouble(double def, double min, double max) const
{
	std::istringstream iss(value);
	iss.imbue(stdLocale);
	double retv;
	if ((iss >> retv).fail()) return def;
	if (retv < min) return min;
	if (retv > max) return max;
	return retv;
}

/**
 * Convert this scalar to a floating-point value.
 * @param def The default value if the value cannot be converted to a float.
 * @param min Minimum clamp value.
 * @param max Maximum clamp value.
 * @return The converted value.
 */
float ScalarNode::AsFloat(float def, float min, float max) const
{
	std::istringstream iss(value);
	iss.imbue(stdLocale);
	float retv;
	if ((iss >> retv).fail()) return def;
	if (retv < min) return min;
	if (retv > max) return max;
	return retv;
}

/**
 * Convert this scalar to an integer value.
 * @param def The default value if the value cannot be converted to a integer.
 * @param min (Optional) Minimum clamp value.
 * @param max (Optinoal) Maximum clamp value.
 * @return The converted value.
 */
int ScalarNode::AsInt(int def, int min, int max) const
{
	std::istringstream iss(value);
	iss.imbue(stdLocale);
	int retv;
	if ((iss >> retv).fail()) return def;
	if (retv < min) return min;
	if (retv > max) return max;
	return retv;
}

/**
 * Convert this scalar to a path.
 * @return The path.
 */
OS::path_t ScalarNode::AsPath() const
{
	return OS::path_t((const OS::path_t::value_type*)Str::UP(value));
}
