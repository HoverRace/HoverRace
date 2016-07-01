
// Button.cpp
//
// Copyright (c) 2013-2016 Michael Imamura.
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

#include "../Control/Action.h"
#include "../Util/Config.h"
#include "BaseContainer.h"
#include "FillBox.h"
#include "Label.h"
#include "Picture.h"
#include "UiFont.h"

#include "Button.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

namespace {

const double DEFAULT_PADDING_HEIGHT = 10;
const double DEFAULT_PADDING_WIDTH = 30;
const double DEFAULT_ICON_GAP = 10;
const double DEFAULT_PRESS_DEPTH = 4;

}  // namespace

namespace {

/**
 * Determine the position coordinates for a widget.
 * @param x The upper-left corner of the bounding box.
 * @param y The upper-left corner of the bounding box.
 * @param w The width of the bounding box.
 * @param h The height of the bounding box.
 * @param alignment The alignment of the contained widget.
 * @return The position.
 */
Vec2 AlignToBounds(
	double x, double y, double w, double h,
	UiViewModel::Alignment alignment)
{
	using Alignment = UiViewModel::Alignment;

	switch (alignment) {
		case Alignment::NW: return { x, y };
		case Alignment::N: return { x + (w / 2.0), y };
		case Alignment::NE: return { x + w, y };
		case Alignment::E: return { x + w, y + (h / 2.0) };
		case Alignment::SE: return { x + w, y + h };
		case Alignment::S: return { x + (w / 2.0), y + h };
		case Alignment::SW: return { x, y + h };
		case Alignment::W: return { x, y + (h / 2.0) };
		case Alignment::CENTER: return { x + (w / 2.0), y + (h / 2.0) };
		default:
			throw Exception("Unknown alignment: " +
				boost::lexical_cast<std::string>(static_cast<int>(alignment)));
	}
}

}  // namespace

/**
 * Constructor for automatically-sized button.
 * @param display The display child elements will be attached to.
 * @param text The text for the button.
 * @param layoutFlags Optional layout flags.
 */
Button::Button(Display &display, const std::string &text,
	uiLayoutFlags_t layoutFlags) :
	SUPER(display, layoutFlags),
	paddingTop(DEFAULT_PADDING_HEIGHT), paddingRight(DEFAULT_PADDING_WIDTH),
	paddingBottom(DEFAULT_PADDING_HEIGHT), paddingLeft(DEFAULT_PADDING_WIDTH),
	iconGap(DEFAULT_ICON_GAP)
{
	Init(text);
}

/**
 * Constructor for fixed-sized button.
 * @param display The display child elements will be attached to.
 * @param size The fixed button size.
 * @param text The text for the button.
 * @param layoutFlags Optional layout flags.
 */
Button::Button(Display &display, const Vec2 &size, const std::string &text,
	uiLayoutFlags_t layoutFlags) :
	SUPER(display, size, layoutFlags),
	paddingTop(DEFAULT_PADDING_HEIGHT), paddingRight(DEFAULT_PADDING_WIDTH),
	paddingBottom(DEFAULT_PADDING_HEIGHT), paddingLeft(DEFAULT_PADDING_WIDTH)
{
	Init(text);
}

Button::~Button()
{
}

void Button::Init(const std::string &text)
{
	const auto &s = display.styles;

	label.reset(new Label(text, s.formFont, s.formFg));
	label->SetAlignment(Alignment::CENTER);
	label->AttachView(display);

	background.reset(new FillBox(0, 0, s.buttonBg));
	background->AttachView(display);

	highlight.reset(new FillBox(0, 0, 0x00000000));
	highlight->AttachView(display);
	highlight->SetBorder(4);
	highlight->SetBorderColor(s.buttonFocusedBg);
}

bool Button::OnNavigate(const Control::Nav &nav)
{
	if (nav.AsDigital() != Control::Nav::NEUTRAL) {
		RelinquishFocus(nav);
		return true;
	}
	else {
		return false;
	}
}

void Button::Layout()
{
	const auto &size = GetSize();
	const auto &s = display.styles;

	background->SetSize(size);
	if (contents) {
		contents->SetSize(size);
	}
	highlight->SetSize(size);

	Vec2 boundsPos(paddingLeft, paddingTop);
	Vec2 boundsSize(
		size.x - (paddingLeft + paddingRight),
		size.y - (paddingTop + paddingBottom));

	// If there is an icon, position it and shift the text to make room.
	if (icon) {
		double iconY = paddingTop;
		if (IsPressed()) {
			iconY += DEFAULT_PRESS_DEPTH;
		}
		icon->SetPos(paddingLeft, iconY);
		double iw = icon->Measure().x + iconGap;
		boundsPos.x += iw;
		boundsSize.x -= iw;
	}

	Vec2 labelPos = AlignToBounds(
		boundsPos.x, boundsPos.y,
		boundsSize.x, boundsSize.y,
		label->GetAlignment());

	double pressOffset = 0;

	Color bgColor;
	if (!IsEnabled()) {
		bgColor = s.buttonDisabledBg;
		background->SetPos(0, 0);
		label->SetColor(s.formDisabledFg);
		label->SetPos(labelPos);
		if (contents) {
			contents->SetPos(0, 0);
		}
		highlight->SetPos(0, 0);
	}
	else if (IsPressed()) {
		pressOffset = DEFAULT_PRESS_DEPTH;
		bgColor = s.buttonPressedBg;
		background->SetPos(0, pressOffset);
		label->SetColor(s.formFg);
		label->SetPos(labelPos.x, labelPos.y + pressOffset);
		if (contents) {
			contents->SetPos(0, pressOffset);
		}
		highlight->SetPos(0, pressOffset);
	}
	else {
		bgColor = s.buttonBg;
		background->SetPos(0, 0);
		label->SetColor(s.formFg);
		label->SetPos(labelPos);
		if (contents) {
			contents->SetPos(0, 0);
		}
		highlight->SetPos(0, 0);
	}

	if (picture) {
		picture->SetPos(0, pressOffset);
		picture->SetSize(size);
	}
	background->SetColor(bgColor);
}

const std::string &Button::GetText() const
{
	return label->GetText();
}

void Button::SetText(const std::string &text)
{
	label->SetText(text);
}

UiViewModel::Alignment Button::getTextAlignment() const
{
	return label->GetAlignment();
}

/**
 * Sets the alignment of the text within the button.
 * @param textAlignment The text alignment.
 */
void Button::SetTextAlignment(Alignment textAlignment)
{
	if (label->GetAlignment() != textAlignment) {
		label->SetAlignment(textAlignment);
		RequestLayout();
	}
}

/**
 * Set the optional icon to appear on the button, in addition to the text.
 *
 * The icon will be sized to the standard size for the button (namely, the
 * height of the text), so while it is safe to share the icon between
 * buttons, be aware that it may be modified.
 *
 * @param icon A subclass of FillBox to use as an icon, or @c nullptr to
 *             unset the icon.
 */
void Button::SetIcon(std::shared_ptr<Box> icon)
{
	if (this->icon != icon) {
		this->icon = std::move(icon);
		FireModelUpdate(Props::ICON);
		RequestSizing();
		RequestLayout();
	}
}

std::shared_ptr<Res<Texture>> Button::ShareTexture() const
{
	if (picture) {
		return picture->ShareTexture();
	}
	else {
		return {};
	}
}

/**
 * Set the background image.
 * @param image The image, or @c nullptr to unset the background image.
 */
void Button::SetTexture(std::shared_ptr<Res<Texture>> image)
{
	Res<Texture> *curTexture = nullptr;
	if (picture) {
		curTexture = picture->ShareTexture().get();
	}

	if (curTexture != image.get()) {
		if (!image) {
			picture.reset();
		}
		else {
			picture.reset(new Picture(image, 1, 1));
			picture->AttachView(display);
		}
		FireModelUpdate(Props::IMAGE);
		RequestLayout();
	}
}

/**
 * Set a container of widgets to render as the contents of the button.
 *
 * This allows for more advanced customization of the rendering of the button.
 *
 * @param contents The container containing the contents (may be @c nullptr).
 */
void Button::SetContents(std::shared_ptr<BaseContainer> contents)
{
	if (this->contents != contents) {
		this->contents = contents;
		FireModelUpdate(Props::CONTENTS);
		RequestLayout();
	}
}

/**
 * Enable auto-sizing on the height only (i.e. fixed width, auto height).
 * This only has an effect if auto-sizing is enabled.
 * @param width The width.
 */
void Button::SetFixedWidth(double width)
{
	if (!fixedWidth || *fixedWidth != width) {
		fixedWidth = width;
		RequestSizing();
		FireModelUpdate(SUPER::Props::SIZE);
	}
}

Vec3 Button::Measure()
{
	const Vec3 labelSize = label->Measure();
	double iconPart = 0;
	if (icon) {
		// The icon's width may vary depending on the height, so set the
		// height now and measure the new width.
		icon->AdjustHeight(labelSize.y);
		iconPart = icon->Measure().x + iconGap;
	}

	if (IsAutoSize()) {
		if (fixedWidth) {
			return {
				*fixedWidth,
				labelSize.y + paddingTop + paddingBottom,
				0 };
		}
		else {
			return {
				labelSize.x + paddingLeft + paddingRight + iconPart,
				labelSize.y + paddingTop + paddingBottom,
				0 };
		}
	}
	else {
		return SUPER::Measure();
	}
}

void Button::FireModelUpdate(int prop)
{
	switch (prop) {
		case UiViewModel::Props::FOCUSED:
		case SUPER::Props::SIZE:
		case SUPER::Props::ENABLED:
		case SUPER::Props::PRESSED:
			RequestLayout();
	}
	SUPER::FireModelUpdate(prop);
}

}  // namespace Display
}  // namespace HoverRace
