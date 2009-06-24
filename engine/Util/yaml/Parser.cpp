
// yaml/Parser.cpp
// Implementation for the LibYAML parser wrapper.
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

#include "NodeFactory.h"

#include "Parser.h"

using namespace yaml;

/**
 * Create a new parser.
 * @param file An open file for reading (may not be null).
 *             It is the caller's job to close the file when the emitter
 *             is destroyed.
 */
Parser::Parser(FILE *file) :
	doc(NULL), root(NULL)
{
	yaml_event_t event;
	int yr;

	// Initialize the LibYAML parser.
	memset(&parser, 0, sizeof(parser));
	yaml_parser_initialize(&parser);
	yaml_parser_set_input_file(&parser, file);

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
	if (node == NULL) {
		Cleanup();
		throw EmptyDocParserExn("Empty document");
	}
	root = NodeFactory::MakeNode(doc, node);
}

/// Destructor.
Parser::~Parser()
{
	Cleanup();
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

