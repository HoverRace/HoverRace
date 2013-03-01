
// SdlLabelView.cpp
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

#include <SDL/SDL.h>

#ifdef WITH_SDL_PANGO
#	include <glib.h>
#	include <SDL_Pango.h>
#endif

#include "../../Util/SelFmt.h"
#include "../../Util/Str.h"
#include "../Label.h"

#include "SdlLabelView.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {
namespace SDL {

#ifdef _WIN32
static RGBQUAD RGB_BLACK = { 0, 0, 0, 0 };
static RGBQUAD RGB_WHITE = { 0xff, 0xff, 0xff, 0 };
#endif

SdlLabelView::SdlLabelView(SdlDisplay &disp, Label &model) :
	SUPER(disp, model),
	surface(), width(0), height(0)
{
}

SdlLabelView::~SdlLabelView()
{
	if (surface) {
		SDL_FreeSurface(surface);
	}
}

void SdlLabelView::OnModelUpdate(int prop)
{
	switch (prop) {
		case Label::Props::COLOR:
		case Label::Props::FONT:
		case Label::Props::TEXT:
			if (surface) {
				SDL_FreeSurface(surface);
				surface = nullptr;
			}
	}
}

void SdlLabelView::PrepareRender()
{
	//TODO: Check if model has changed.
	if (!surface) Update();
}

void SdlLabelView::Render()
{
	disp.DrawUiSurface(surface, model.GetPos());
}

void SdlLabelView::Update()
{
	if (surface) SDL_FreeSurface(surface);

	SDL_Surface *tempSurface;

#	ifdef WITH_SDL_PANGO
		const std::string &s = model.GetText();
		char *escapedBuf = g_markup_escape_text(s.c_str(), -1);

		std::ostringstream oss;
		oss << SelFmt<SEL_FMT_PANGO> <<
			"<span font=\"" << model.GetFont() << "\" "
			"color=\"" << model.GetColor() << "\">" <<
			escapedBuf << "</span>";

		g_free(escapedBuf);

		//TODO: Reuse the context.
		SDLPango_Context *ctx = SDLPango_CreateContext();
		SDLPango_SetDpi(ctx, 60, 60);
		SDLPango_SetDefaultColor(ctx, MATRIX_TRANSPARENT_BACK_WHITE_LETTER);
		SDLPango_SetMinimumSize(ctx, 0, 0);
		SDLPango_SetMarkup(ctx, oss.str().c_str(), -1);

		width = SDLPango_GetLayoutWidth(ctx);
		height = SDLPango_GetLayoutHeight(ctx);

		//TODO: Handle text effect.
		realWidth = width;
		realHeight = height;

		// Draw the text onto an SDL surface.
		tempSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
			width, height, 32,
			(MR_UInt32)(255 << (8 * 3)),
			(MR_UInt32)(255 << (8 * 2)),
			(MR_UInt32)(255 << (8 * 1)),
			255);
		SDLPango_Draw(ctx, tempSurface, 0, 0);

		SDLPango_FreeContext(ctx);

#	elif defined(_WIN32)
		HDC hdc = CreateCompatibleDC(NULL);

		const UiFont font = model.GetFont();
		HFONT stdFont = CreateFontW(
			//TODO: Support fractional sizes.
			static_cast<int>(font.size),
			0, 0, 0,
			(font.style & UiFont::BOLD) ? FW_BOLD : FW_NORMAL,
			(font.style & UiFont::ITALIC) ? TRUE : FALSE,
			0, 0, 0, 0, 0,
			ANTIALIASED_QUALITY,
			0,
			Str::UW(font.name));
		HFONT oldFont = (HFONT)SelectObject(hdc, stdFont);

		const wchar_t *ws = model.GetWText().c_str();
		const size_t wsLen = model.GetWText().size();

		// Get the dimensions required for the font.
		RECT sz;
		memset(&sz, 0, sizeof(sz));
		DrawTextW(hdc, ws, wsLen, &sz, DT_CALCRECT | DT_NOPREFIX);
		width = sz.right - sz.left;
		height = sz.bottom - sz.top;

		// Create a 32-bit DIB to draw the text onto.
		BITMAPINFO *bmpInfo =
			(BITMAPINFO*)malloc(sizeof(BITMAPINFOHEADER) + 2 * sizeof(RGBQUAD));
		BITMAPINFOHEADER &bmih = bmpInfo->bmiHeader;
		memset(&bmih, 0, sizeof(bmih));
		bmih.biSize = sizeof(bmih);
		bmih.biWidth = width;
		bmih.biHeight = -height;
		bmih.biPlanes = 1;
		bmih.biBitCount = 32;
		bmih.biCompression = BI_RGB;

		MR_UInt32 *bits;
		HBITMAP bmp = CreateDIBSection(hdc, bmpInfo, DIB_RGB_COLORS,
			(void**)&bits, NULL, 0);
		HBITMAP oldBmp = (HBITMAP)SelectObject(hdc, bmp);

		// Draw the text.
		// Note that we draw the text as white so we can use it as the
		// alpha channel (we blend in the color when copying to the SDL
		// surface).
		SetTextColor(hdc, RGB(0xff, 0xff, 0xff));
		SetBkColor(hdc, RGB(0, 0, 0));
		SetBkMode(hdc, OPAQUE);
		DrawTextW(hdc, ws, wsLen, &sz, DT_NOCLIP | DT_NOPREFIX);

		//TODO: Handle text effect.
		realWidth = width;
		realHeight = height;

		tempSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
			width, height, 32,
			(MR_UInt32)(255 << (8 * 3)),
			(MR_UInt32)(255 << (8 * 2)),
			(MR_UInt32)(255 << (8 * 1)),
			255);

		// Note: The alpha component of the color is currently ignored.
		const Color c = model.GetColor();
		MR_UInt8 cr = c.bits.r;
		MR_UInt8 cg = c.bits.g;
		MR_UInt8 cb = c.bits.b;

		// Now copy from the bitmap into our image buffer.
		// DIB rows are 32-bit word-aligned.
		char buf[9] = { 0 };
		int destSkip = realWidth - width;
		MR_UInt32 *src = bits;
		MR_UInt8 *dest = (MR_UInt8*)(tempSurface->pixels);
		memset(dest, 0, tempSurface->h * tempSurface->pitch);
		for (int y = 0; y < height; ++y) {
			MR_UInt8 *destRow = dest;
			for (int x = 0; x < width; ++x) {
				MR_UInt32 px = *src++;

				MR_UInt8 alpha = px & 0xff;
				px = ((MR_UInt32)(cr * alpha / 255) << 24) +
					((MR_UInt32)(cg * alpha / 255) << 16) +
					((MR_UInt32)(cb * alpha / 255) << 8) +
					alpha;

				*((MR_UInt32*)dest) = px;
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
		throw UnimplementedExn("SdlLabelView::Update");
#	endif

	// Convert the surface to the display format.
	surface = SDL_DisplayFormatAlpha(tempSurface);
	SDL_FreeSurface(tempSurface);
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
