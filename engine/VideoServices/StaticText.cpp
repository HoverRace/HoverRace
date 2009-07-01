
// StaticText.cpp
// Static text widget with i18n support.
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

#include "../Util/Str.h"
#include "2DViewPort.h"

#include "StaticText.h"

using namespace HoverRace::Util;

using namespace HoverRace::VideoServices;

#ifdef _WIN32
static RGBQUAD RGB_BLACK = { 0, 0, 0, 0 };
static RGBQUAD RGB_WHITE = { 0xff, 0xff, 0xff, 0 };
#endif

/**
 * Constructor.
 * @param s The text (UTF-8).
 * @param font The font name.
 * @param size The text size.
 * @param bold @c true for bold text.
 * @param italic @c true for italic text.
 * @param color The color index.
 * @param effect Effect to apply.
 */
StaticText::StaticText(const std::string &s,
                       const std::string &font,
                       int size, bool bold, bool italic,
                       MR_UInt8 color, effect_t effect) :
	s(s), ws(Str::Utf8ToWide(s.c_str())), wsLen(wcslen(ws)),
	font(font, size, bold, italic), color(color), effect(effect),
	bitmap(NULL), width(0), height(0)
{
	Update();
}

/**
 * Constructor.
 * @param s The text (UTF-8).
 * @param font The font.
 * @param color The color index.
 * @param effect Effect to apply.
 */
StaticText::StaticText(const std::string &s,
                       const HoverRace::VideoServices::Font &font,
                       MR_UInt8 color, effect_t effect) :
	s(s), ws(Str::Utf8ToWide(s.c_str())), wsLen(wcslen(ws)),
	font(font), color(color), effect(effect),
	bitmap(NULL), width(0), height(0), realWidth(0), realHeight(0)
{
	Update();
}


StaticText::~StaticText()
{
	free(bitmap);
	free(ws);
}

int StaticText::GetWidth() const
{
	return width;
}

int StaticText::GetHeight() const
{
	return height;
}

int StaticText::GetRealWidth() const
{
	return realWidth;
}

int StaticText::GetRealHeight() const
{
	return realHeight;
}

void StaticText::SetText(const std::string &s)
{
	if (this->s != s) {
		this->s = s;
		free(ws);
		ws = Str::Utf8ToWide(s.c_str());
		wsLen = wcslen(ws);
		Update();
	}
}

void StaticText::SetFont(const HoverRace::VideoServices::Font &font)
{
	if (!(this->font == font)) {
		this->font = font;
		Update();
	}
}

void StaticText::SetColor(MR_UInt8 color)
{
	this->color = color;
}

void StaticText::Update()
{
	free(bitmap);

#ifdef _WIN32
	HDC hdc = CreateCompatibleDC(NULL);

	HFONT stdFont = CreateFont(
		font.size,
		0, 0, 0,
		font.bold ? FW_BOLD : FW_NORMAL,
		font.italic ? TRUE : FALSE,
		0, 0, 0, 0, 0, 0, 0,
		font.name.c_str());
	HFONT oldFont = (HFONT)SelectObject(hdc, stdFont);

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

	switch (effect) {
		case StaticText::EFFECT_SHADOW:
			realWidth = width + 1;
			realHeight = height + 1;
			break;
		default:
			realWidth = width;
			realHeight = height;
	}

	// Now copy from the bitmap into our image buffer.
	// DIB rows are 32-bit word-aligned.
	bitmap = (MR_UInt8*)malloc(realWidth * realHeight);
	memset(bitmap, 0, realWidth * realHeight);
	int padding = 4 - (width & 3);
	if (padding == 4) padding = 0;
	int destSkip = realWidth - width;
	MR_UInt8 *src = bits;
	MR_UInt8 *dest = bitmap;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			*dest++ = (*src++ > 0) ? 0xff : 0x00;
		}
		dest += destSkip;
		src += padding;
	}

	SelectObject(hdc, oldBmp);
	DeleteObject(bmp);
	free(bmpInfo);

	SelectObject(hdc, oldFont);
	DeleteObject(stdFont);

	DeleteDC(hdc);
#endif

	// Apply effect (we might refactor this out).
	switch (effect) {
		case EFFECT_SHADOW: ApplyShadow(); break;
	}
}

/// Apply the drop shadow effect.
void StaticText::ApplyShadow()
{
	int skip = realWidth - width;
	MR_UInt8 *src = bitmap;
	MR_UInt8 *dest = bitmap + realWidth + 1;

	for (int y = 1; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			if (*dest == 0 && *src == 0xff) {
				*dest++ = 0x01;
			}
			else {
				++dest;
			}
			++src;
		}
		dest += skip;
		src += skip;
	}
}

/**
 * Draw the text onto the viewport.
 * @param x The X coordinate (upper-left corner of text,
 *          or middle of text if @e centerX is set).
 * @param y The Y coordinate (upper-left corner of text).
 * @param centerX Center the text on the X coordinate.
 * @param vp The viewport (may not be @c NULL).
 */
void StaticText::Blt(int x, int y, MR_2DViewPort *vp, bool centerX) const
{
#ifdef _WIN32
	if (centerX) x -= width / 2;

	int dw = vp->GetXRes();
	int dh = vp->GetYRes();
	int sx1, sy1, sx2, sy2;
	int dx1, dy1;

	// Check if the text is completely off the screen.
	if (x >= dw || y >= dh || x + realWidth <= 0 || y + realHeight <= 0) return;

	// Clip the text to the viewport.
	if (x < 0) { sx1 = -x; dx1 = 0; } else { sx1 = 0; dx1 = x; }
	if (y < 0) { sy1 = -y; dy1 = 0; } else { sy1 = 0; dy1 = y; }
	sx2 = (realWidth - sx1 + dx1 >= dw) ? (sx1 + dw - dx1) : realWidth;
	sy2 = (realHeight - sy1 + dy1 >= dh) ? (sy1 + dh - dy1) : realHeight;

	int rowLen = vp->GetLineLen();
	MR_UInt8 *destRow = vp->GetBuffer() + (rowLen * dy1 + dx1);
	MR_UInt8 *dest = destRow;
	for (int y = sy1; y < sy2; ++y) {
		for (int x = sx1; x < sx2; ++x) {
			switch (bitmap[y * realWidth + x]) {
				case 0x00: break;
				case 0x01: *dest = 0x1a; break;
				default:   *dest = color; break;
			}
			++dest;
		}
		dest = (destRow += rowLen);
	}
#endif
}

