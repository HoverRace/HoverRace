
// SdlDynamicTexture.cpp
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

#include "../../Util/MR_Types.h"

#include "SdlDynamicTexture.h"

namespace HoverRace {
namespace Display {
namespace SDL {

namespace {

/**
 * Initializes the surface based on an existing texture.
 * @param texture The texture (may not be @c nullptr).
 * @return @c nullptr if it fails, else a SDL_Surface*.
 */
SDL_Surface *InitSurface(SDL_Texture *texture)
{
	if (!texture) {
		throw Exception("Cannot create dynamic null texture.");
	}

	MR_UInt32 fmt;
	int w, h;
	if (SDL_QueryTexture(texture, &fmt, nullptr, &w, &h) < 0) {
		throw DynamicTextureExn(
			std::string("Failed to query source for dynamic texture: ") +
			SDL_GetError());
	}

	int bpp;
	MR_UInt32 rm, gm, bm, am;
	if (SDL_PixelFormatEnumToMasks(fmt, &bpp, &rm, &gm, &bm, &am) == SDL_FALSE)
	{
		throw DynamicTextureExn(
			std::string("Invalid pixel format for dynamic texture: ") +
			SDL_GetError());
	}

	SDL_Surface *retv;
	if ((retv = SDL_CreateRGBSurface(0, w, h, bpp, rm, gm, bm, am)) == nullptr)
	{
		throw DynamicTextureExn(
			std::string("Failed to create surface for dynamic texture: ") +
			SDL_GetError());
	}

	return retv;
}

}  // namespace

/**
 * Constructor.
 * @param display The display.
 * @param texture The texture (may not be @c nullptr).
 * @throws DynamicTextureExn Could not be created
 *                           (@p texture will still be freed).
 */
SdlDynamicTexture::SdlDynamicTexture(SdlDisplay &display,
	SDL_Texture *texture) :
	SUPER(display, texture)
{
	try {
		surface = InitSurface(texture);
	}
	catch (...) {
		if (texture) SDL_DestroyTexture(texture);
	}
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
