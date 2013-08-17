
// SdlSurfaceText.cpp
//
// Copyright (c) 2013 Michael Imamura.
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

#include <SDL2/SDL.h>

#ifdef WITH_SDL_PANGO
#	include <glib.h>
#	include <SDL_Pango.h>
#elif defined(WITH_SDL_TTF)
#	include <SDL2/SDL_ttf.h>
#endif

#include "../../Util/SelFmt.h"
#include "../../Util/Str.h"
#include "../../Exception.h"
#include "../Label.h"

#include "SdlSurfaceText.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {
namespace SDL {

SdlSurfaceText::SdlSurfaceText() :
	font(), color(COLOR_WHITE), wrapWidth(-1),
	width(0), height(0)
{
}

/**
 * Create a new SDL surface and render text onto it.
 * The new SDL surface will be sized to fit the rendered text.
 * @param s The text to render (may not be blank).
 * @return The newly-created surface.
 */
SDL_Surface *SdlSurfaceText::RenderToNewSurface(
#	ifdef _WIN32
		const std::wstring &s
#	else
		const std::string &s
#	endif
	)
{
	SDL_Surface *tempSurface = nullptr;

	bool fixedWidth = wrapWidth > 0;

#	ifdef WITH_SDL_PANGO
		/*TODO: Update to SDL2_Pango when it's available.
		char *escapedBuf = g_markup_escape_text(s.c_str(), -1);

		std::ostringstream oss;
		oss << SelFmt<SEL_FMT_PANGO> <<
			"<span font=\"" << font << "\">" <<
			escapedBuf << "</span>";

		g_free(escapedBuf);

		SDLPango_Context *ctx = disp.GetPangoContext();
		SDLPango_SetMinimumSize(ctx, 0, 0);
		SDLPango_SetMarkup(ctx, oss.str().c_str(), -1);

		width = SDLPango_GetLayoutWidth(ctx);
		height = SDLPango_GetLayoutHeight(ctx);

		// Draw the text onto an SDL surface.
		tempSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
			width, height, 32,
			(MR_UInt32)(255 << (8 * 3)),
			(MR_UInt32)(255 << (8 * 2)),
			(MR_UInt32)(255 << (8 * 1)),
			255);
		SDLPango_Draw(ctx, tempSurface, 0, 0);
		*/
		throw UnimplementedExn("SdlSurfaceText::RenderToNewSurface for SDL_Pango");

#	elif defined(WITH_SDL_TTF)
		TTF_Font *ttfFont = disp.LoadTtfFont(font);

		//TODO: Handle newlines ourselves.
		SDL_Color color = { 0xff, 0xff, 0xff };

		tempSurface = TTF_RenderUTF8_Blended_Wrapped(ttfFont,
			s.c_str(), color,
			fixedWidth ? wrapWidth : 4096);
		width = tempSurface->w;
		height = tempSurface->h;

#	elif defined(_WIN32)
		HDC hdc = CreateCompatibleDC(NULL);

		// We will scale the viewport to be 1px = 100 viewport units.
		SetGraphicsMode(hdc, GM_ADVANCED);
		XFORM xform;
		memset(&xform, 0, sizeof(xform));
		xform.eM11 = 0.01f;
		xform.eM22 = 0.01f;
		SetWorldTransform(hdc, &xform);

		font.size *= 100.0;

		HFONT stdFont = CreateFontW(
			static_cast<int>(font.size),
			0, 0, 0,
			(font.style & UiFont::BOLD) ? FW_BOLD : FW_NORMAL,
			(font.style & UiFont::ITALIC) ? TRUE : FALSE,
			0, 0, 0, 0, 0,
			ANTIALIASED_QUALITY,
			0,
			Str::UW(font.name));
		HFONT oldFont = (HFONT)SelectObject(hdc, stdFont);

		const wchar_t *ws = s.c_str();
		const size_t wsLen = s.size();

		// Get the dimensions required for the font.
		RECT sz;
		memset(&sz, 0, sizeof(sz));
		UINT fmtFlags = 0;
		if (fixedWidth) {
			sz.right = wrapWidth * 100;
			fmtFlags |= DT_WORDBREAK;
		}
		DrawTextW(hdc, ws, wsLen, &sz, fmtFlags | DT_CALCRECT | DT_NOPREFIX);
		width = (sz.right - sz.left) / 100;
		height = (sz.bottom - sz.top) / 100;

		// If the calculated width is larger than the fixed width, then we'll
		// intentionally create a large-height DIB and let the final DrawText()
		// call tell us how high the rendered text was.
		bool lateHeightCalc = fixedWidth && width > wrapWidth;
		if (lateHeightCalc) {
			width = wrapWidth;
			sz.right = width * 100;
		}

		// Create a 32-bit DIB to draw the text onto.
		BITMAPINFO *bmpInfo =
			(BITMAPINFO*)malloc(sizeof(BITMAPINFOHEADER) + 2 * sizeof(RGBQUAD));
		BITMAPINFOHEADER &bmih = bmpInfo->bmiHeader;
		memset(&bmih, 0, sizeof(bmih));
		bmih.biSize = sizeof(bmih);
		bmih.biWidth = width;
		bmih.biHeight = lateHeightCalc ? -4096 : -height;
		bmih.biPlanes = 1;
		bmih.biBitCount = 32;
		bmih.biCompression = BI_RGB;

		MR_UInt32 *bits;
		HBITMAP bmp = CreateDIBSection(hdc, bmpInfo, DIB_RGB_COLORS,
			(void**)&bits, NULL, 0);
		HBITMAP oldBmp = (HBITMAP)SelectObject(hdc, bmp);

		// Draw the text.
		// Note that we draw the text as white so we can use it as the
		// alpha channel.
		SetTextColor(hdc, RGB(0xff, 0xff, 0xff));
		SetBkColor(hdc, RGB(0, 0, 0));
		SetBkMode(hdc, OPAQUE);
		int rendHeight = DrawTextW(hdc, ws, wsLen, &sz, fmtFlags | DT_NOCLIP | DT_NOPREFIX);

		if (lateHeightCalc) {
			height = rendHeight / 100;
		}

		tempSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
			width, height, 32,
			(MR_UInt32)(255 << (8 * 3)),
			(MR_UInt32)(255 << (8 * 2)),
			(MR_UInt32)(255 << (8 * 1)),
			255);

		// Now copy from the bitmap into our image buffer.
		// DIB rows are 32-bit word-aligned.
		char buf[9] = { 0 };
		int destSkip = 0;
		MR_UInt32 *src = bits;
		MR_UInt8 *dest = (MR_UInt8*)(tempSurface->pixels);
		memset(dest, 0, tempSurface->h * tempSurface->pitch);
		for (int y = 0; y < height; ++y) {
			MR_UInt8 *destRow = dest;
			for (int x = 0; x < width; ++x) {
				const MR_UInt32 px = *src++;
				*((MR_UInt32*)dest) = 0xffffff00 + (px & 0xff);
				dest += 4;
			}
			dest = destRow + tempSurface->pitch;
		}

		SelectObject(hdc, oldBmp);
		DeleteObject(bmp);
		free(bmpInfo);

		SelectObject(hdc, oldFont);
		DeleteObject(stdFont);

		DeleteDC(hdc);

#	else
		throw UnimplementedExn("SdlSurfaceText::RenderToNewSurface");
#	endif

	return tempSurface;
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
