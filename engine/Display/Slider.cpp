
// Slider.cpp
//
// Copyright (c) 2014, 2015 Michael Imamura.
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
#include "../Util/Log.h"
#include "UiFont.h"
#include "FillBox.h"
#include "Label.h"
#include "RuleLine.h"

#include "Slider.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

/**
 * Constructor.
 * @param display The display child elements will be attached to.
 * @param min The minimum value.
 * @param max The maximum value.
 * @param step The smallest unit between selectable values.
 * @param layoutFlags Optional layout flags.
 */
Slider::Slider(Display &display, double min, double max, double step,
	uiLayoutFlags_t layoutFlags) :
	SUPER(display, layoutFlags),
	min(min), max(max), step(step),
	value(min)
{
	assert(min <= max);

	const auto &s = display.styles;

	background.reset(new FillBox(0, 0, s.buttonBg));
	background->AttachView(display);

	indicator.reset(new FillBox(0, 0, s.buttonBg));
	indicator->AttachView(display);

	if (min <= 0 && max >= 0) {
		zeroLine.reset(new RuleLine(RuleLine::Direction::V, 1, 1, s.formFg));
		zeroLine->AttachView(display);
	}
}

Slider::~Slider()
{
}

bool Slider::OnNavigate(const Control::Nav &nav)
{
	using Nav = Control::Nav;

	auto dir = nav.AsDigital();
	switch (dir) {
		case Nav::NEUTRAL:
			return true;

		case Nav::UP:
			//TODO: Relinquish focus.
			return true;
		case Nav::DOWN:
			//TODO: Relinquish focus.
			return true;

		case Nav::LEFT:
			SetValue(value - step);
			return true;
		case Nav::RIGHT:
			SetValue(value + step);
			return true;

		default:
			HR_LOG(warning) << "Unhandled navigation direction: " << dir;
			return false;
	}
}

void Slider::OnMouseDrag(const Vec2 &relPos)
{
	SetValue(((relPos.x * (max - min)) / GetSize().x) + min);
}

/**
 * Set the value.
 *
 * The value will be clamped to the range of the slider.
 *
 * @param value The value.
 */
void Slider::SetValue(double value)
{
	if (value < min) value = min;
	else if (value > max) value = max;
	else {
		// Clamp to the nearest step.
		value = round(value * (1.0 / step)) * step;
	}

	if (this->value != value) {
		this->value = value;
		FireModelUpdate(Props::VALUE);
		valueChangedSignal(value);
		RequestLayout();
	}
}

void Slider::Layout()
{
	const auto &size = GetSize();
	double w = size.x;
	const auto &s = display.styles;

	background->SetSize(size);
	
	double scale = w / (max - min);
	double offset, len;
	if (min > 0) {
		len = (value - min) * scale;
		offset = 0;
	}
	else if (max < 0) {
		len = -(value - max) * scale;
		offset = w - len;
	}
	else {
		double zeroPoint = -min * scale;
		zeroLine->SetLength(size.y);
		zeroLine->SetPos(zeroPoint, 0);

		if (value < 0) {
			len = -value * scale;
			offset = zeroPoint - len;
		}
		else if (value > 0) {
			len = value * scale;
			offset = zeroPoint;
		}
		else {
			len = 0;
			offset = zeroPoint;
		}
	}

	indicator->SetPos(offset, 0);
	indicator->SetSize(len, size.y);

	if (!IsEnabled()) {
		background->SetColor(s.buttonDisabledBg);
		indicator->SetColor(s.buttonDisabledBg);
	}
	else if (IsPressed()) {
		background->SetColor(s.buttonPressedBg);
		indicator->SetColor(s.buttonPressedBg);
	}
	else {
		background->SetColor(s.buttonBg);
		indicator->SetColor(s.buttonBg);
	}
}

Vec3 Slider::Measure()
{
	return SUPER::Measure();
}

void Slider::FireModelUpdate(int prop)
{
	switch (prop) {
		case SUPER::Props::SIZE:
		case SUPER::Props::ENABLED:
		case SUPER::Props::PRESSED:
			RequestLayout();
	}
	SUPER::FireModelUpdate(prop);
}

}  // namespace Display
}  // namespace HoverRace
