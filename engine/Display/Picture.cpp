
// Picture.cpp
//
// Copyright (c) 2014, 2016 Michael Imamura.
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

#include "Picture.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

/**
 * Constructor.
 * @param texture The texture (may be @c nullptr).
 * @param size The size of the box, where @c x is the width
 *             and @c y is the height.
 * @param color The color of the box (including alpha).
 * @param layoutFlags Optional layout flags.
 */
Picture::Picture(std::shared_ptr<Res<Texture>> texture, const Vec2 &size,
	Color color, uiLayoutFlags_t layoutFlags) :
	SUPER(size, layoutFlags),
	texture(std::move(texture)), color(color)
{
}

/**
 * Constructor.
 * @param texture The texture (may be @c nullptr).
 * @param w The width of the box.
 * @param h The height of the box.
 * @param color The color of the box (including alpha).
 * @param layoutFlags Optional layout flags.
 */
Picture::Picture(std::shared_ptr<Res<Texture>> texture, double w, double h,
	Color color, uiLayoutFlags_t layoutFlags) :
	Picture(texture, { w, h }, color, layoutFlags)
{
}

/**
 * Set the texture.
 * @param texture The texture (may be @c nullptr).
 */
void Picture::SetTexture(std::shared_ptr<Res<Texture>> texture)
{
	if (this->texture != texture) {
		this->texture = std::move(texture);
		FireModelUpdate(Props::TEXTURE);
	}
}

/**
 * Set the color of the icon.
 * @param color The color (including alpha).
 */
void Picture::SetColor(const Color color)
{
	if (this->color != color) {
		this->color = color;
		FireModelUpdate(Props::COLOR);
	}
}

}  // namespace Display
}  // namespace HoverRace
