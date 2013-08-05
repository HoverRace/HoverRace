
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

#include <SDL2/SDL.h>

#ifdef WITH_SDL_PANGO
#	include <glib.h>
#	include <SDL_Pango.h>
#elif defined(WITH_SDL_TTF)
#	include <SDL2/SDL_ttf.h>
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
	texture(), colorChanged(true), width(0), height(0)
{
	uiScaleChangedConnection = disp.GetUiScaleChangedSignal().connect(
		std::bind(&SdlLabelView::OnUiScaleChanged, this));
}

SdlLabelView::~SdlLabelView()
{
	uiScaleChangedConnection.disconnect();
	if (texture) {
		SDL_DestroyTexture(texture);
	}
}

void SdlLabelView::OnModelUpdate(int prop)
{
	switch (prop) {
		case Label::Props::COLOR:
			colorChanged = true;
			break;

		case Label::Props::FONT:
		case Label::Props::TEXT:
			if (texture) {
				SDL_DestroyTexture(texture);
				texture = nullptr;
			}
			break;
	}
}

void SdlLabelView::OnUiScaleChanged()
{
	if (texture) {
		SDL_DestroyTexture(texture);
		texture = nullptr;
	}
}

void SdlLabelView::PrepareRender()
{
	if (!texture) {
		UpdateTexture();
	} else if (colorChanged) {
		UpdateTextureColor();
	}
}

void SdlLabelView::Render()
{
	if (model.GetText().empty()) return;

	int w, h;
	SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
	disp.DrawUiTexture(texture,
		model.GetAlignedPos(unscaledWidth, unscaledHeight),
		model.GetLayoutFlags());
}

/**
 * Update the state when the text is an empty string.
 * In this case, we measure the height of the string, and set the width to be 1
 * (to prevent accidental divisions by zero).
 */
void SdlLabelView::UpdateBlank()
{
	double scale = 1.0;

	UiFont font = model.GetFont();
	if (!model.IsLayoutUnscaled()) {
		font.size *= (scale = disp.GetUiScale());
	}

#	ifdef WITH_SDL_PANGO
		throw UnimplementedExn("SdlLabelView::UpdateBlank for SDL_Pango");

#	elif defined(WITH_SDL_TTF)
		TTF_Font *ttfFont = disp.LoadTtfFont(font);
		realWidth = width = 1;
		realHeight = height = TTF_FontHeight(ttfFont);

#	elif defined(_WIN32)
		HDC hdc = CreateCompatibleDC(NULL);

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

		RECT sz;
		memset(&sz, 0, sizeof(sz));
		DrawTextW(hdc, L" ", 1, &sz, DT_CALCRECT | DT_NOPREFIX);
		realWidth = width = 1;
		realHeight = height = sz.bottom - sz.top;

		SelectObject(hdc, oldFont);
		DeleteObject(stdFont);

		DeleteDC(hdc);

#	else
		throw UnimplementedExn("SdlLabelView::UpdateBlank");
#	endif

	unscaledWidth = width / scale;
	unscaledHeight = height / scale;

	UpdateTextureColor();
}

void SdlLabelView::UpdateTexture()
{
	if (texture) SDL_DestroyTexture(texture);

	if (model.GetText().empty()) {
		UpdateBlank();
		return;
	}

	SDL_Surface *tempSurface;
	double scale = 1.0;

	UiFont font = model.GetFont();
	if (!model.IsLayoutUnscaled()) {
		font.size *= (scale = disp.GetUiScale());
	}

	bool fixedWidth = !model.IsAutoWidth();
	double uiWrapWidth = model.GetWrapWidth();
	int wrapWidth = model.IsLayoutUnscaled() ?
		(int)uiWrapWidth :
		(int)(uiWrapWidth * scale);

#	ifdef WITH_SDL_PANGO
		const std::string &s = model.GetText();
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

#	elif defined(WITH_SDL_TTF)
		TTF_Font *ttfFont = disp.LoadTtfFont(font);

		//TODO: Handle newlines ourselves.
		SDL_Color color = { 0xff, 0xff, 0xff };

		tempSurface = TTF_RenderUTF8_Blended_Wrapped(ttfFont,
			model.GetText().c_str(), color,
			fixedWidth ? wrapWidth : 4096);
		realWidth = width = tempSurface->w;
		realHeight = height = tempSurface->h;

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

		const wchar_t *ws = model.GetWText().c_str();
		const size_t wsLen = model.GetWText().size();

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

		//TODO: Handle text effect.
		realWidth = width;
		realHeight = height;

		tempSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
			width, height, 32,
			(MR_UInt32)(255 << (8 * 3)),
			(MR_UInt32)(255 << (8 * 2)),
			(MR_UInt32)(255 << (8 * 1)),
			255);

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
		throw UnimplementedExn("SdlLabelView::UpdateTexture");
#	endif

	// Convert the surface to the display format.
	texture = SDL_CreateTextureFromSurface(disp.GetRenderer(), tempSurface);
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
	SDL_FreeSurface(tempSurface);

	// We pre-scale the texture (by adjusting the font size) so that would
	// normally throw off the size adjustments later, so we need to keep track
	// of what the size would be (approximately) if we hadn't pre-scaled the
	// texture.
	unscaledWidth = width / scale;
	unscaledHeight = height / scale;

	UpdateTextureColor();
}

void SdlLabelView::UpdateTextureColor()
{
	if (!model.GetText().empty()) {
		const Color cm = model.GetColor();
		SDL_SetTextureColorMod(texture, cm.bits.r, cm.bits.g, cm.bits.b);
		SDL_SetTextureAlphaMod(texture, cm.bits.a);
	}

	colorChanged = false;
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
