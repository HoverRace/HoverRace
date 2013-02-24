
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

#include "../Util/SelFmt.h"
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

void SdlLabelView::PrepareRender()
{
	//TODO: Check if model has changed.
	if (!surface) Update();
}

void SdlLabelView::Render()
{
	if (surface) {
		SDL_BlitSurface(surface, nullptr, SDL_GetVideoSurface(), nullptr);
	}
}

void SdlLabelView::Update()
{
	if (surface) SDL_FreeSurface(surface);

#	ifdef WITH_SDL_PANGO
		const std::string &s = model.GetText();
		char *escapedBuf = g_markup_escape_text(s.c_str(), -1);

		std::ostringstream oss;
		oss << SelFmt<SEL_FMT_PANGO> <<
			"<span font=\"" << model.GetFont() << "\">" <<
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
		surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
			width, height, 32,
			(MR_UInt32)(255 << (8 * 3)),
			(MR_UInt32)(255 << (8 * 2)),
			(MR_UInt32)(255 << (8 * 1)),
			255);
		SDLPango_Draw(ctx, surface, 0, 0);

		SDLPango_FreeContext(ctx);

#	elif defined(_WIN32)
		HDC hdc = CreateCompatibleDC(NULL);

		const VideoServices::FontSpec font = model.GetFont();
		HFONT stdFont = CreateFont(
			font.size,
			0, 0, 0,
			font.bold ? FW_BOLD : FW_NORMAL,
			font.italic ? TRUE : FALSE,
			0, 0, 0, 0, 0, 0, 0,
			font.name.c_str());
		HFONT oldFont = (HFONT)SelectObject(hdc, stdFont);

		const wchar_t *ws = model.GetWText().c_str();
		const size_t wsLen = model.GetWText().size();

		// Get the dimensions required for the font.
		RECT sz;
		memset(&sz, 0, sizeof(sz));
		DrawTextW(hdc, ws, wsLen, &sz, DT_CALCRECT | DT_NOPREFIX);
		width = sz.right - sz.left;
		height = sz.bottom - sz.top;

		// Create a monochrome DIB to draw the text onto.
		BITMAPINFO *bmpInfo =
			(BITMAPINFO*)malloc(sizeof(BITMAPINFOHEADER) + 2 * sizeof(RGBQUAD));
		BITMAPINFOHEADER &bmih = bmpInfo->bmiHeader;
		memset(&bmih, 0, sizeof(bmih));
		bmih.biSize = sizeof(bmih);
		bmih.biWidth = width;
		bmih.biHeight = -height;
		bmih.biPlanes = 1;
		bmih.biBitCount = 8;
		bmih.biCompression = BI_RGB;
		bmih.biClrUsed = 2;
		bmih.biClrImportant = 2;
		bmpInfo->bmiColors[0] = RGB_BLACK;
		bmpInfo->bmiColors[1] = RGB_WHITE;

		MR_UInt8 *bits;
		HBITMAP bmp = CreateDIBSection(hdc, bmpInfo, DIB_RGB_COLORS,
			(void**)&bits, NULL, 0);
		HBITMAP oldBmp = (HBITMAP)SelectObject(hdc, bmp);

		// Draw the text.
		SetTextColor(hdc, RGB(0xff, 0xff, 0xff));
		SetBkColor(hdc, RGB(0, 0, 0));
		SetBkMode(hdc, OPAQUE);
		DrawTextW(hdc, ws, wsLen, &sz, DT_NOCLIP | DT_NOPREFIX);

		//TODO: Handle text effect.
		realWidth = width;
		realHeight = height;

		surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
			width, height, 32,
			(MR_UInt32)(255 << (8 * 3)),
			(MR_UInt32)(255 << (8 * 2)),
			(MR_UInt32)(255 << (8 * 1)),
			255);

		// Now copy from the bitmap into our image buffer.
		// DIB rows are 32-bit word-aligned.
		int padding = 4 - (width & 3);
		if (padding == 4) padding = 0;
		int destSkip = realWidth - width;
		MR_UInt8 *src = bits;
		MR_UInt8 *dest = (MR_UInt8*)(surface->pixels);
		memset(dest, 0, surface->h * surface->pitch);
		for (int y = 0; y < height; ++y) {
			MR_UInt8 *destRow = dest;
			for (int x = 0; x < width; ++x) {
				*((MR_UInt32*)dest) = (*src++ > 0) ? 0xffffffff : 0x00000000;
				dest += 4;
			}
			dest = destRow + surface->pitch;
			src += padding;
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
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
