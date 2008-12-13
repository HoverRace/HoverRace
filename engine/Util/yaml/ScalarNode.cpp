
/* yaml/ScalarNode.cpp
	Wrapper for LibYAML scalar nodes. */

#include "stdafx.h"

#include <sstream>

#include "ScalarNode.h"

using namespace yaml;

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
	if (stricmp(value, "y") == 0 ||
		stricmp(value, "yes") == 0 ||
		stricmp(value, "true") == 0 ||
		stricmp(value, "on") == 0)
	{
		return true;
	}
	if (stricmp(value, "n") == 0 ||
		stricmp(value, "no") == 0 ||
		stricmp(value, "false") == 0 ||
		stricmp(value, "off") == 0)
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
	int retv;
	if ((iss >> retv).fail()) return def;
	if (retv < min) return min;
	if (retv > max) return max;
	return retv;
}
