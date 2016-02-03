
// Parser.h
//
// Copyright (c) 2008, 2009, 2015-2016 Michael Imamura.
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

namespace HoverRace {
namespace Util {
namespace yaml {

/// Standard exception thrown for parser errors.
class ParserExn : public YamlExn
{
	using SUPER = YamlExn;

public:
	ParserExn() : SUPER() { }
	ParserExn(const char *const &s) : SUPER(s) { }
	virtual ~ParserExn() noexcept { }
};

/// Exception for when the document is empty or missing the header.
class EmptyDocParserExn : public ParserExn
{
	using SUPER = ParserExn;

public:
	EmptyDocParserExn() : SUPER() { }
	EmptyDocParserExn(const char *const &s) : SUPER(s) { }
	virtual ~EmptyDocParserExn() noexcept { }
};

/// Wrapper for the LibYAML parser.
class Parser
{
private:
	Parser() = delete;
public:
	Parser(FILE *file);
	Parser(std::istream &is);
	virtual ~Parser();

protected:
	void InitParser();
	void InitStream();

private:
	void Cleanup();

public:
	Node *GetRootNode() const { return root; }

private:
	yaml_parser_t parser;
	yaml_document_t *doc;
	Node *root;
};

}  // namespace yaml
}  // namespace Util
}  // namespace HoverRace

