
// yaml/MapNode.h
// Header for yaml::MapNode.
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

#pragma once

#include <map>

#include "Node.h"

namespace yaml
{
	class MapNode : public Node
	{
		typedef Node SUPER;

		public:
			MapNode(yaml_document_t *doc, yaml_node_t *node);
			virtual ~MapNode();

		private:
			void Init() const;

			typedef std::map<std::string,Node*> children_t;

		public:
			Node *Get(const std::string &key);

			// STL-like iteration, so we can use BOOST_FOREACH on the node itself.
			typedef children_t::iterator iterator;
			typedef children_t::const_iterator const_iterator;
			typedef children_t::value_type value_type;
			const_iterator begin() const;
			iterator begin();
			const_iterator end() const;
			iterator end();

		private:
			mutable children_t *children;
	};
}
