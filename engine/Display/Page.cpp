
// Page.cpp
//
// Copyright (c) 2014 Michael Imamura.
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

#include <rapidxml/rapidxml.hpp>

#include "../Util/Config.h"
#include "../Util/Log.h"

#include "Page.h"

namespace xml = rapidxml;

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

class Page::Processor /*{{{*/
{
	public:
		Processor(Page &page, const xml::xml_node<> &root,
			const Style &initStyle);

	private:
		void ProcessNode(const xml::xml_node<> &node);
		void ProcessElement(const xml::xml_node<> &node);
		void ProcessChildren(const xml::xml_node<> &node);

	private:
		Page &page;
		std::stack<Style> styles;
}; //}}}

Page::Page(const std::string &src)
{
	ParseSource(src);
}

void Page::ParseSource(const std::string &src)
{
	xml::xml_document<> doc;

	// RapidXml modifies the source string, so allocate a new string tied to
	// the lifetime of the xml_document.
	size_t len = src.length();
	char *srcbuf = doc.allocate_string(nullptr, len + 6 + 7 + 1);
	memcpy(srcbuf, "<page>", 6);
	memcpy(srcbuf + 6, src.c_str(), len);
	memcpy(srcbuf + len + 6, "</page>", 7 + 1);  // Include zero terminator.

	try {
		doc.parse<0>(srcbuf);
	} catch (xml::parse_error &ex) {
		Log::Error("Unable to parse page: %s", ex.what());
		return;
	}

	Config *cfg = Config::GetInstance();
	Style defaultStyle(UiFont(cfg->GetDefaultFontName()));

	Processor(*this, *(doc.first_node()), defaultStyle);
}

void Page::AddChunk(const Chunk &chunk)
{
	Log::Info("Adding chunk: %s", chunk.text.c_str());
}

//{{{ Page::Processor //////////////////////////////////////////////////////////

Page::Processor::Processor(Page &page, const xml::xml_node<> &root,
                           const Style &initStyle) :
	page(page)
{
	styles.emplace(initStyle);
	ProcessChildren(root);
}

void Page::Processor::ProcessNode(const xml::xml_node<> &node)
{
	switch (node.type()) {
		case xml::node_data:
		case xml::node_cdata:
			page.AddChunk(Chunk(node.value(), styles.top()));
			break;

		case xml::node_element:
			ProcessElement(node);
			break;

		default:
			// Ignore everything else.
			break;
	}
}

void Page::Processor::ProcessElement(const xml::xml_node<> &node)
{
	styles.push(styles.top());
	Style &style = styles.top();

	// Inspired by HTML 1.0.
	std::string name = node.name();
	if (name == "i") {
		style.font.style ^= UiFont::ITALIC;
	}
	else if (name == "b") {
		style.font.style ^= UiFont::BOLD;
	}
	else {
		Log::Info("Ignoring unhandled element: %s", name.c_str());
	}

	ProcessChildren(node);

	styles.pop();
}

void Page::Processor::ProcessChildren(const xml::xml_node<> &node)
{
	xml::xml_node<> *n = node.first_node();
	while (n) {
		ProcessNode(*n);
		n = n->next_sibling();
	}
}

//}}} Page::Processor

}  // namespace Display
}  // namespace HoverRace
