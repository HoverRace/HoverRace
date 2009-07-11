
// MultipartText.cpp
// Text widget with static and dynamic formatted portions.
//
// Copyright (c) 2009 Michael Imamura.
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

#ifndef _WIN32
#	include <stdarg.h>
#endif

#include <boost/format.hpp>

#include "Font.h"
#include "NumericGlyphs.h"
#include "StaticText.h"

#include "MultipartText.h"

using namespace HoverRace::VideoServices;

/// Base class for individual text parts.
class MultipartText::Part
{
	MultipartText &container;

	public:
		Part(MultipartText &container) : container(container) { }
		virtual ~Part() { }

	protected:
		MultipartText &GetContainer() const
		{
			return container;
		}

	public:
		/**
		 * Render the part.
		 * After the part is rendered, the (X,Y) coordinates will point to
		 * the position of the next part.
		 * @param[in,out] x The starting X coordinate.
		 * @param[in,out] y The starting Y coordinate.
		 * @param vp The viewport (may not be @c NULL).
		 */
		virtual void Blt(int &x, int &y, MR_2DViewPort *vp) = 0;

		/**
		 * Provide arguments for the part to optionally consume.
		 * This function will always be called before Blt.
		 * @param vl The arg list.
		 */
		virtual void ConsumeArg(va_list vl) { }
};

/// Constant string part.
class MultipartText::StringPart : public MultipartText::Part
{
	typedef MultipartText::Part SUPER;

	std::string s;
	StaticText *widget;

	public:
		StringPart(MultipartText &container) :
			SUPER(container), s(), widget(NULL) { }

		virtual ~StringPart()
		{
			delete widget;
		}

		virtual void Blt(int &x, int &y, MR_2DViewPort *vp)
		{
			if (widget == NULL) {
				MultipartText &container = GetContainer();
				widget = new StaticText(s, container.GetFont(), container.GetColor());
			}
			widget->Blt(x, y, vp);
			x += widget->GetWidth();
		}

		void Append(const char *s)
		{
			this->s += s;
		}
};

/// Dynamic number part.
class MultipartText::FormatPart : public MultipartText::Part
{
	typedef MultipartText::Part SUPER;
	
	boost::format fmt;
	std::string s;
	double curVal;
	StaticText *widget;

	public:
		FormatPart(MultipartText &container, const boost::format &fmt) :
			SUPER(container), fmt(fmt), s(), curVal(0.0), widget(NULL) { }

		virtual ~FormatPart() {
			delete widget;
		}

		virtual void Blt(int &x, int &y, MR_2DViewPort *vp)
		{
			if (widget != NULL) {
				widget->Blt(x, y, vp);
				x += widget->GetWidth();
			}
			else {
				const NumericGlyphs *glyphs = GetContainer().GetGlyphs();
				for (std::string::const_iterator iter = s.begin();
					iter != s.end(); ++iter)
				{
					const StaticText *glyph = glyphs->GetGlyph(*iter);
					if (glyph != NULL) {
						glyph->Blt(x, y, vp);
						x += glyph->GetWidth();
					}
				}
			}
		}

		virtual void ConsumeArg(va_list vl)
		{
			// We always take a double for now.
			// In the future we might try checking the format to support
			// other types.
			double val = va_arg(vl, double);
			if (s.empty() || curVal != val) {
				curVal = val;
				s = boost::str(fmt % val);

				MultipartText &container = GetContainer();
				if (container.GetGlyphs() == NULL) {
					// Use StaticText directly.
					if (widget == NULL) {
						widget = new StaticText(s, container.GetFont(),
							container.GetColor());
					}
					else {
						widget->SetText(s);
					}
				}
			}
		}
};

/**
 * Constructor.
 * @param font The text font.
 * @param glyphs Pointer to the collection of numeric glyphs for dynamic parts.
 *               (may be @c NULL if a StaticText should be created for dynamic parts).
 * @param color The text color.
 */
MultipartText::MultipartText(const HoverRace::VideoServices::Font &font,
                             const HoverRace::VideoServices::NumericGlyphs *glyphs,
                             MR_UInt8 color) :
	font(font), glyphs(glyphs), color(color),
	parts(), lastStringPart(NULL)
{
}

MultipartText::~MultipartText()
{
	for (parts_t::iterator iter = parts.begin(); iter != parts.end(); ++iter) {
		delete *iter;
	}
}

const HoverRace::VideoServices::Font &MultipartText::GetFont() const
{
	return font;
}

const NumericGlyphs *MultipartText::GetGlyphs() const
{
	return glyphs;
}

const MR_UInt8 MultipartText::GetColor() const
{
	return color;
}

/**
 * Draw the text onto the viewport.
 * @param x The X coordinate (upper-left corner of text).
 * @param y The Y coordinate (upper-left corner of text).
 * @param vp The viewport (may not be @c NULL).
 * @param ... Variable list of arguments, one per dynamic part.
 */
void MultipartText::Blt(int x, int y, MR_2DViewPort *vp, ...) const
{
	va_list vl;

	va_start(vl, vp);
	for (parts_t::const_iterator iter = parts.begin();
		iter != parts.end(); ++iter)
	{
		Part *part = *iter;
		part->ConsumeArg(vl);
		part->Blt(x, y, vp);
	}
	va_end(vl);
}

/**
 * Add a new StringPart or append to an existing StringPart.
 * This is an optimization so that consecutive strings are merged into one.
 * @param s The string (may not be @c NULL or empty).
 */
void MultipartText::AddStringPart(const char *s)
{
	if (lastStringPart == NULL) {
		lastStringPart = new StringPart(*this);
		parts.push_back(lastStringPart);
	}
	lastStringPart->Append(s);
}

/**
 * Add a non-string part.
 * @param part The part to add (may not be @c NULL).
 */
void MultipartText::AddPart(Part *part)
{
	lastStringPart = NULL;
	parts.push_back(part);
}

MultipartText &HoverRace::VideoServices::operator<<(MultipartText &self, const char *s)
{
	if (s != NULL && s[0] != '\0') self.AddStringPart(s);
	return self;
}

MultipartText &HoverRace::VideoServices::operator<<(MultipartText &self, const std::string &s)
{
	if (!s.empty()) self.AddStringPart(s.c_str());
	return self;
}

MultipartText &HoverRace::VideoServices::operator<<(MultipartText &self, const boost::format &fmt)
{
	self.AddPart(new MultipartText::FormatPart(self, fmt));
	return self;
}
