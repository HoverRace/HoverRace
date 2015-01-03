
// Counter.cpp
//
// Copyright (c) 2013-2015 Michael Imamura.
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

#include "FillBox.h"
#include "Label.h"

#include "Counter.h"

#ifdef _WIN32
#	define isnan _isnan
#endif

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

namespace {
	const Color BG_COLOR = 0x3f000000;
	const Color TITLE_COLOR = 0xffe3063e;
	const Color VALUE_COLOR = TITLE_COLOR;
	const double MARGIN_WIDTH = 15;
	const double MARGIN_HEIGHT = 10;
}

/// Indicator that the total should not be displayed.
const double Counter::NO_TOTAL = std::numeric_limits<double>::quiet_NaN();

/**
 * Constructor.
 * @param display The display child elements will be attached to.
 * @param title The label text for the counter.
 * @param initValue The initial value for the counter.
 * @param total The optional total value (may be NO_TOTAL to hide the total).
 */
Counter::Counter(Display &display, const std::string &title, double initValue,
                 double total) :
	SUPER(display),
	value(initValue), total(total),
	valueChanged(true)
{
	const auto &s = display.styles;

	bg = NewChild<FillBox>(0, 0, BG_COLOR);

	titleLbl = NewChild<Label>(title, s.hudNormalHeadFont, TITLE_COLOR);
	titleLbl->SetAlignment(Alignment::W);

	valueLbl = NewChild<Label>("", s.hudNormalFont, VALUE_COLOR);
}

void Counter::OnHudRescaled(const Vec2 &hudScale)
{
	const auto &s = display.styles;
	bool small = hudScale.x < 1.0 || hudScale.y < 1.0;

	if (small) {
		titleLbl->SetFont(s.hudSmallHeadFont);
		valueLbl->SetFont(s.hudSmallFont);
	}
	else {
		titleLbl->SetFont(s.hudNormalHeadFont);
		valueLbl->SetFont(s.hudNormalFont);
	}

	RequestLayout();
}

/**
 * Set the current value.
 * @param value The value.
 */
void Counter::SetValue(double value)
{
	if (this->value != value) {
		this->value = value;
		valueChanged = true;
	}
}

/**
 * Set the total value.
 * @param total The total value, or NO_TOTAL to hide the total.
 */
void Counter::SetTotal(double total)
{
	if (this->total != total) {
		this->total = total;
		valueChanged = true;
	}
}

/**
 * Check if the total value is hidden from display.
 * @return @c true if the total value is hidden.
 */
bool Counter::IsTotalHidden() const
{
	return isnan(total) != 0;
}

/**
 * Hide the total value from being displayed.
 */
void Counter::HideTotal()
{
	SetTotal(NO_TOTAL);
}

void Counter::Layout()
{
	SUPER::Layout();

	const Vec3 titleSize = titleLbl->Measure();
	const Vec3 valueSize = valueLbl->Measure();

	titleLbl->SetPos(MARGIN_WIDTH, MARGIN_HEIGHT + (valueSize.y / 2.0));

	double x = MARGIN_WIDTH;
	if (!titleLbl->GetText().empty()) {
		x += titleSize.x + 10.0;
	}
	valueLbl->SetPos(x, MARGIN_HEIGHT);

	Vec2 sz(x + valueSize.x + MARGIN_WIDTH,
		MARGIN_HEIGHT + valueSize.y + MARGIN_HEIGHT);
	bg->SetSize(sz);
	SetSize(sz);
}

void Counter::Advance(Util::OS::timestamp_t)
{
	static boost::format VALUE_FMT("%0.9g");
	static boost::format TOTAL_FMT("%0.9g / %0.9g");

	if (valueChanged) {
		if (IsTotalHidden()) {
			valueLbl->SetText(boost::str(VALUE_FMT % value));
		}
		else {
			valueLbl->SetText(boost::str(TOTAL_FMT % value % total));
		}
		RequestLayout();
		valueChanged = false;
	}
}

}  // namespace Display
}  // namespace HoverRace
