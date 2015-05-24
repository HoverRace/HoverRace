
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

#include <bitset>

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
	spaceWidth(MeasureSpaceWidth(display, font)),
	curMap(0), curX(0), curY(0)
{
}

SdlTypeCase::~SdlTypeCase()
{
	HR_LOG(debug) << "Type case [" << font << "] destroyed.";
}

MR_UInt32 SdlTypeCase::CountTextures() const
{
	// Safe cast since we constrain the maximum number of maps.
	return static_cast<MR_UInt32>(maps.size());
}

/**
 * Measure the width of a space.
 *
 * We don't actually render spaces, so we need to know how wide it is for
 * layout purposes.
 *
 * @param display The SDL display for rendering.
 * @param font The font.
 * @return The width, in pixels.
 */
int SdlTypeCase::MeasureSpaceWidth(SdlDisplay &display, const UiFont &font)
{
	TTF_Font *ttfFont = display.LoadTtfFont(font, false);
	int advance = 0;
	if (TTF_GlyphMetrics(ttfFont, ' ', nullptr, nullptr, nullptr, nullptr,
		&advance) < 0)
	{
		return 0;
	}
	return advance;
}

/**
 * Adds a glyph to the backing textures.
 * @param [in,out] ent The glyph entry to initialize.
 * @param s A single UTF-8 character.
 * @param cp The Unicode code point represented by the character.
 * @param added Buffer of added glyphs.
 * @return The same glyph entry that was passed in.
 */
GlyphEntry &SdlTypeCase::AddGlyph(GlyphEntry &ent, const std::string &s,
	MR_UInt32 cp, std::string &added)
{
	TTF_Font *ttfFont = display.LoadTtfFont(font, false);
	SDL_Color color = { 0xff, 0xff, 0xff, 0xff };
	SDL_Surface *src = TTF_RenderUTF8_Blended(ttfFont, s.c_str(), color);
	if (!src) {
		// No need to log the error; SDL_ttf will log the error itself.
		ent.page = 0;
		auto &rect = ent.srcRect;
		rect.x = 0;
		rect.y = 0;
		rect.w = 0;
		rect.h = 0;
		return ent;
	}
	SDL_SetSurfaceBlendMode(src, SDL_BLENDMODE_NONE);
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

	ent.cp = cp;
	ent.page = curMap;
	auto &rect = ent.srcRect;
	rect.x = curX;
	rect.y = curY;
	rect.w = w;
	rect.h = h;

	curX += w + 1;

	added += s;

	return ent;
}

/**
 * Finds a glyph in the backing textures, creating it if necessary.
 * @param s A single UTF-8 character.
 * @param cp The Unicode code point represented by the character.
 * @param added Buffer of added glyphs.
 * @return The entry for the glyph.
 */
GlyphEntry &SdlTypeCase::FindGlyph(const std::string &s, MR_UInt32 cp,
	std::string &added)
{
	if (cp > 65535) {
		HR_LOG(warning) << "AddGlyph(): code point (" << cp << ") > 65535";
		cp = 0;
	}

	// Find the GlyphEntry for this glyph.
	// Glyphs are organized into "pages" of 256 glyphs.
	MR_UInt32 pageIdx = cp / 256;
	auto &page = glyphs[pageIdx];
	if (!page) {
		page.reset(new glyphPage_t());
	}
	GlyphEntry &ent = (*page)[cp % 256];

	return ent.IsInitialized() ? ent : AddGlyph(ent, s, cp, added);
}

void SdlTypeCase::Prepare(const std::string &s, TypeLine *rects)
{
	if (s.empty()) return;

	std::string buf;

	HR_LOG(trace) << "Preparing: " << s;

	if (rects) {
		rects->typeCase = shared_from_this();
		rects->glyphs.clear();
	}

	std::string added;
	auto prev = s.begin();
	auto iter = prev;
	auto end = s.end();
	int sx = 0;
	int cx = 0, cy = 0;
	try {
		while (iter != end) {
			MR_UInt32 cp = utf8::next(iter, end);
			buf.assign(prev, iter);
			prev = iter;

			HR_LOG(trace) << "Finding: " << cp;

			switch (cp) {
				case '\r':
					// Ignore CR; only LF is used for newlines.
					break;

				case ' ':
					cx += spaceWidth;
					sx += spaceWidth;
					break;

				case '\n':
					cx = 0;
					cy += fontHeight;
					break;

				default:
					auto &entry = FindGlyph(buf, cp, added);
					if (rects) {
						const auto &gr = entry.srcRect;
						rects->glyphs.emplace_back(&entry,
							SDL_Rect{ cx, cy, gr.w, gr.h });
						cx += gr.w;
						sx += entry.srcRect.w;
					}
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

	if (rects) {
		rects->width = sx;
		rects->height = cy + fontHeight;
	}

	// All maps that need update will be at the end, so update in reverse
	// order until we find one that didn't need updating.
	for (auto iter = maps.rbegin(); iter != maps.rend(); ++iter) {
		if (!(*iter)->Update()) break;
	}

	if (!added.empty()) {
		HR_LOG(debug) << "Type case [" << font << "] added "
			"[" << utf8::distance(added.cbegin(), added.cend()) << "] "
			"glyphs: " << added;
	}
}

void SdlTypeCase::Render(const TypeLine &s, const Color cm, int x, int y)
{
#	ifdef _DEBUG
		if (s.typeCase.get() != this) {
			throw Exception("TypeLine owner mismatch.");
		}
#	endif

	std::bitset<MAX_MAPS> usedTextures;
	usedTextures.reset();

	auto renderer = display.GetRenderer();
	SDL_Rect destRect = { x, y, 0, 0 };
	for (const auto &glyph : s.glyphs) {
		auto page = glyph.first->page;
		SDL_Texture *texture = maps[page].get()->Get();

		if (!usedTextures[page]) {
			SDL_SetTextureColorMod(texture, cm.bits.r, cm.bits.g, cm.bits.b);
			SDL_SetTextureAlphaMod(texture, cm.bits.a);
			SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
			usedTextures.set(page);
		}

		const auto &srcRect = glyph.first->srcRect;
		destRect.x = x + glyph.second.x;
		destRect.y = y + glyph.second.y;
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

	SDL_Texture *texture = maps[idx]->Get();
	SDL_SetTextureColorMod(texture, 0xff, 0xff, 0xff);
	SDL_SetTextureAlphaMod(texture, 0xff);
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

	SDL_RenderCopy(renderer, texture, nullptr, &destRect);
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
