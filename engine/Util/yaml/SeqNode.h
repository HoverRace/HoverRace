
// yaml/SeqNode.h
// Header for yaml::SeqNode.
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

#include "Node.h"

#include <vector>

namespace yaml
{
	class SeqNode : public Node
	{
		typedef Node SUPER;

		public:
			SeqNode(yaml_document_t *doc, yaml_node_t *node);
			virtual ~SeqNode();

		private:
			void Init() const;

			typedef std::vector<Node*> children_t;

		public:
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
