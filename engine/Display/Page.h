
// Page.h
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

#pragma once

#include "Color.h"
#include "UiFont.h"

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
	namespace Display {
		class Display;
	}
}

namespace HoverRace {
namespace Display {

/**
 * Formatted text parser.
 * @author Michael Imamura
 */
class MR_DllDeclare Page
{
	public:
		Page(const std::string &src);
		~Page() { }

	private:
		class Processor;

		struct Style
		{
			Style(const UiFont &font, Color color) :
				font(font), color(color) { }
			Style(const Style &o) :
				font(o.font), color(o.color) { }
			Style(Style &&o) :
				font(std::move(o.font)), color(o.color) { }

			Style &operator=(const Style &o)
			{
				font = o.font;
				color = o.color;
				return *this;
			}

			Style &operator=(Style &&o)
			{
				font = std::move(o.font);
				color = o.color;
				return *this;
			}

			UiFont font;
			Color color;
		};

		struct Chunk
		{
			Chunk(const std::string &text, const Style &style) :
				text(text), style(style) { }
			Chunk(const Chunk &o) :
				text(o.text), style(o.style) { }
			Chunk(Chunk &&o) :
				text(std::move(o.text)), style(std::move(o.style)) { }

			Chunk &operator=(const Chunk &o)
			{
				text = o.text;
				style = o.style;
				return *this;
			}

			Chunk &operator=(Chunk &&o)
			{
				text = std::move(o.text);
				style = std::move(o.style);
				return *this;
			}

			std::string text;
			Style style;
		};

	private:
		void ParseSource(const std::string &src);
		void AddChunk(const std::string &text, const Style &style);

	private:
		std::string src;
		std::vector<Chunk> chunks;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
