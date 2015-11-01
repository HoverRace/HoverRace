
// MapNode.h
//
// Copyright (c) 2008, 2009, 2015 Michael Imamura.
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

#pragma once

#include <limits.h>

#include "../OS.h"

#include "Node.h"

namespace HoverRace {
namespace Util {
namespace yaml {

class MapNode : public Node
{
	using SUPER = Node;

public:
	MapNode(yaml_document_t *doc, yaml_node_t *node);
	virtual ~MapNode();

private:
	void Init() const;

	using children_t = std::map<std::string, Node*>;

public:
	Node *Get(const std::string &key) const;

	void ReadString(const std::string &key, std::string &dest) const;
	void ReadBool(const std::string &key, bool &dest) const;
	void ReadDouble(const std::string &key, double &dest,
		double min, double max) const;
	void ReadFloat(const std::string &key, float &dest,
		float min, float max) const;
	void ReadInt(const std::string &key, int &dest,
		int min = INT_MIN, int max = INT_MAX) const;
	void ReadPath(const std::string &key, OS::path_t &dest) const;

	// STL-like iteration, so we can use range-for on the node itself.
	using iterator = children_t::iterator;
	using const_iterator = children_t::const_iterator;
	using value_type = children_t::value_type;
	const_iterator begin() const;
	iterator begin();
	const_iterator end() const;
	iterator end();

private:
	mutable children_t *children;
};

}  // namespace yaml
}  // namespace Util
}  // namespace HoverRace
