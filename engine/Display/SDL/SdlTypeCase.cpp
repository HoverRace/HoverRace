
// SdlTypeCase.cpp
//
// Copyright (c) 2015 Michael Imamura.
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

#include <utf8/utf8.h>

#include "../../Util/Log.h"
#include "SdlDisplay.h"
#include "SdlDynamicTexture.h"

#include "SdlTypeCase.h"

namespace HoverRace {
namespace Display {
namespace SDL {

namespace {

const MR_UInt32 MAX_MAPS = 32;

}  // namespace

/**
 * Constructor.
 * @param display The SDL display for rendering.
 * @param font The font.
 * @param width The width of the backing texture.
 * @param height The height of the backing texture.
 */
SdlTypeCase::SdlTypeCase(SdlDisplay &display, const UiFont &font,
	int width, int height) :
	SUPER(font, width, height), display(display),
	fontHeight(TTF_FontHeight(display.LoadTtfFont(font, false))),
	curMap(0), curX(0), curY(0)
{
}

SdlTypeCase::~SdlTypeCase()
{
}

MR_UInt32 SdlTypeCase::CountTextures() const
{
	// Safe cast since we constrain the maximum number of maps.
	return static_cast<MR_UInt32>(maps.size());
}

/**
 * Adds a glyph to the backing textures.
 * @param [in,out] ent The glyph entry to initialize.
 * @param s A single UTF-8 character.
 * @return The same glyph entry that was passed in.
 */
GlyphEntry &SdlTypeCase::AddGlyph(GlyphEntry &ent, const std::string &s)
{
	TTF_Font *ttfFont = display.LoadTtfFont(font, false);
	SDL_Color color = { 0xff, 0xff, 0xff, 0xff };
	SDL_Surface *src = TTF_RenderUTF8_Blended(ttfFont, s.c_str(), color);
	int w = src->w;
	int h = src->h;

	if (w > width || h > height) {
		SDL_FreeSurface(src);
		throw Exception(boost::str(boost::format(
			"Glyph size (%dx%d) exceeds texture size (%dx%d).") %
			w % h % width % height));
	}

	if (curX + w >= width) {
		curX = 0;
		if (curY + fontHeight + h >= height) {
			curY = 0;
			curMap++;
			HR_LOG(debug) << "Spilling type case for font [" << font << "] "
				"into new texture (" << curMap << " total, " <<
				width << "x" << height << ")";
		}
		else {
			curY += fontHeight + 1;
		}
	}

	if (curMap == maps.size()) {
		if (curMap == MAX_MAPS) {
			SDL_FreeSurface(src);
			throw Exception(boost::str(boost::format(
				"Exceeded maximum number of textures (%u) for font: %s") %
				MAX_MAPS % font));
		}
		maps.emplace_back(new SdlDynamicTexture(display, width, height));
	}
	auto &map = maps.back();

	HR_LOG(trace) << "Adding glyph [" << s << "] to texture " << curMap <<
		" at " << curX << ", " << curY;

	map->Update([&](SDL_Surface *surface) {
		SDL_Rect destRect = { curX, curY, 0, 0 };
		SDL_BlitSurface(src, nullptr, surface, &destRect);
		return false;  // Update later, when all glyphs have been added.
	});

	SDL_FreeSurface(src);

	ent.page = curMap;
	auto &rect = ent.srcRect;
	rect.x = curX;
	rect.y = curY;
	rect.w = w;
	rect.h = h;

	curX += w + 1;

	return ent;
}

/**
 * Finds a glyph in the backing textures, creating it if necessary.
 * @param s A single UTF-8 character.
 * @param cp The Unicode code point represented by the character.
 * @return The entry for the glyph.
 */
GlyphEntry &SdlTypeCase::FindGlyph(const std::string &s, MR_UInt32 cp)
{
	// Find the GlyphEntry for this glyph.
	if (cp < common.size()) {
		GlyphEntry &ent = common[cp];
		return ent.IsInitialized() ? ent : AddGlyph(ent, s);
	}
	else {
		throw UnimplementedExn(boost::str(
			boost::format("AddGlyph(): code point (%u) > %u") %
				cp % common.size()));
	}
}

void SdlTypeCase::Prepare(const std::string &s, TypeLine *rects)
{
	if (s.empty()) return;

	std::string buf;

	HR_LOG(debug) << "Preparing: " << s;

	if (rects) {
		rects->typeCase = shared_from_this();
		rects->glyphs.clear();
	}

	auto prev = s.begin();
	auto iter = prev;
	auto end = s.end();
	try {
		while (iter != end) {
			MR_UInt32 cp = utf8::next(iter, end);
			buf.assign(prev, iter);
			prev = iter;

			HR_LOG(trace) << "Finding: " << cp;

			auto &entry = FindGlyph(buf, cp);
			if (rects) {
				rects->glyphs.push_back(&entry);
			}
		}
	}
	catch (utf8::not_enough_room &ex) {
		HR_LOG(warning) << "Invalid bytes at the end of the string: " <<
			ex.what();
	}
	catch (utf8::invalid_utf8 &ex) {
		HR_LOG(warning) << "Invalid UTF-8 sequence: " << ex.what();
	}

	// All maps that need update will be at the end, so update in reverse
	// order until we find one that didn't need updating.
	for (auto iter = maps.rbegin(); iter != maps.rend(); ++iter) {
		if (!(*iter)->Update()) break;
	}
}

void SdlTypeCase::Render(const TypeLine &s, int x, int y)
{
	if (s.typeCase.get() != this) {
		throw Exception("TypeLine owner mismatch.");
	}

	auto renderer = display.GetRenderer();
	SDL_Rect destRect = { x, y, 0, 0 };
	for (const auto &glyph : s.glyphs) {
		SDL_Texture *texture = maps[glyph->page].get()->Get();
		const auto &srcRect = glyph->srcRect;
		destRect.w = srcRect.w;
		destRect.h = srcRect.h;
		SDL_RenderCopy(renderer, texture, &srcRect, &destRect);
		destRect.x += srcRect.w;
	}
}

void SdlTypeCase::RenderTexture(MR_UInt32 idx, int x, int y, double scale)
{
	if (idx >= maps.size()) {
		HR_LOG(warning) << "Invalid texture index: " << idx;
		return;
	}

	auto renderer = display.GetRenderer();

	SDL_Rect destRect = { x, y,
		static_cast<int>(scale * width),
		static_cast<int>(scale * height) };

	SDL_RenderCopy(renderer, maps[idx]->Get(), nullptr, &destRect);
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
