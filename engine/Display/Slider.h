
// Slider.h
//
// Copyright (c) 2014 Michael Imamura.
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

#pragma once

#include "ClickRegion.h"

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
	namespace Display {
		class Display;
		class FillBox;
		class Label;
		class RuleLine;
	}
}

namespace HoverRace {
namespace Display {

/**
 * A slider for selecting values from a range.
 * @author Michael Imamura
 */
class MR_DllDeclare Slider : public ClickRegion
{
	using SUPER = ClickRegion;

public:
	struct Props
	{
		enum {
			VALUE = SUPER::Props::NEXT_,
			NEXT_,  ///< First index for subclasses.
		};
	};

public:
	Slider(Display &display, double min, double max, double step,
		uiLayoutFlags_t layoutFlags = 0);
	virtual ~Slider();

public:
	virtual void AttachView(Display &disp) { AttachViewDynamic(disp, this); }

protected:
	void OnMouseDrag(const Vec2 &relPos) override;

public:
	using valueChangedSignal_t = boost::signals2::signal<void(double)>;
	valueChangedSignal_t &GetValueChangedSignal() { return valueChangedSignal; }

public:
	double GetMin() const { return min; }
	double GetMax() const { return max; }
	double GetStep() const { return step; }

	double GetValue() const { return value; }
	void SetValue(double value);

	FillBox *GetBackgroundChild() const { return background.get(); }
	FillBox *GetIndicatorChild() const { return indicator.get(); }
	RuleLine *GetZeroLineChild() const { return zeroLine.get(); }

protected:
	virtual void Layout();

public:
	virtual Vec3 Measure();
	virtual void FireModelUpdate(int prop);

private:
	double min;
	double max;
	double step;
	double value;

	valueChangedSignal_t valueChangedSignal;

	std::unique_ptr<FillBox> background;
	std::unique_ptr<FillBox> indicator;
	std::unique_ptr<RuleLine> zeroLine;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
