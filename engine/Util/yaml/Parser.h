
// yaml/Parser.h
// Header for yaml::Parser.
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

#include <yaml.h>

#include <exception>

#include "Node.h"
#include "YamlExn.h"

namespace yaml
{
	/// Standard exception thrown for parser errors.
	class ParserExn : public YamlExn
	{
		typedef YamlExn SUPER;

		public:
			ParserExn() : SUPER() { }
			ParserExn(const char *const &s) : SUPER(s) { }
			virtual ~ParserExn() throw() { }
	};

	/// Exception for when the document is empty or missing the header.
	class EmptyDocParserExn : public ParserExn
	{
		typedef ParserExn SUPER;

		public:
			EmptyDocParserExn() : SUPER() { }
			EmptyDocParserExn(const char *const &s) : SUPER(s) { }
			virtual ~EmptyDocParserExn() throw() { }
	};

	/// Wrapper for the LibYAML parser.
	class Parser
	{
		private:
			Parser() { }
		public:
			Parser(FILE *file);
			virtual ~Parser();

		private:
			void Cleanup();

		public:
			Node *GetRootNode() const { return root; }

		private:
			yaml_parser_t parser;
			yaml_document_t *doc;
			Node *root;
	};
}
