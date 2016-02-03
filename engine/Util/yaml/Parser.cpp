
// Parser.cpp
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

#include "NodeFactory.h"

#include "Parser.h"

namespace HoverRace {
namespace Util {
namespace yaml {

namespace {

int InputStreamHandler(void *data, unsigned char *buffer, size_t size,
	size_t *sizeRead)
{
	std::istream &is = *static_cast<std::istream*>(data);

	if (is.eof()) {
		// "On EOF, the handler should set the size_read to 0 and return 1."
		sizeRead = 0;
		return 1;
	}
	else if (!is) {
		return 0;
	}

	std::streamsize chunksize;
	if (size > std::numeric_limits<std::streamsize>::max()) {
		chunksize = std::numeric_limits<std::streamsize>::max();
	}
	else {
		chunksize = static_cast<std::streamsize>(size);
	}

	is.read(reinterpret_cast<char*>(buffer), chunksize);
	auto count = is.gcount();
	if (count < 0) return 0;
	*sizeRead = static_cast<size_t>(count);

	return (is.eof() || is) ? 1 : 0;
}

}  // namespace

/**
 * Create a new parser for a file handle.
 * @param file An open file for reading (may not be null).
 *             It is the caller's job to close the file when the emitter
 *             is destroyed.
 */
Parser::Parser(FILE *file) :
	doc(NULL), root(NULL)
{
	InitParser();
	yaml_parser_set_input_file(&parser, file);
	InitStream();
}

/**
 * Create a new parser for an input stream.
 * @param is The input stream.
 */
Parser::Parser(std::istream &is) :
	doc(nullptr), root(nullptr)
{
	InitParser();
	yaml_parser_set_input(&parser, InputStreamHandler, &is);
	InitStream();
}

/// Destructor.
Parser::~Parser()
{
	Cleanup();
}

/// Initialize the LibYAML parser.
void Parser::InitParser()
{
	memset(&parser, 0, sizeof(parser));
	yaml_parser_initialize(&parser);
}

void Parser::InitStream()
{
	yaml_event_t event;
	int yr;

	// Read the header.
	yr = yaml_parser_parse(&parser, &event);
	if (!yr || event.type != YAML_STREAM_START_EVENT) {
		yaml_event_delete(&event);
		Cleanup();
		throw EmptyDocParserExn("Expected stream start");
	}
	yaml_event_delete(&event);

	// Read the document all at once.
	doc = (yaml_document_t*)malloc(sizeof(yaml_document_t));
	if (!yaml_parser_load(&parser, doc)) {
		Cleanup();
		throw ParserExn("Malformed file");
	}

	// Get the root node.
	yaml_node_t *node = yaml_document_get_root_node(doc);
	if (!node) {
		Cleanup();
		throw EmptyDocParserExn("Empty document");
	}
	root = NodeFactory::MakeNode(doc, node);
}

void Parser::Cleanup()
{
	delete root;

	if (doc != NULL) {
		yaml_document_delete(doc);
		free(doc);
	}

	yaml_parser_delete(&parser);
}

}  // namespace yaml
}  // namespace Util
}  // namespace HoverRace

