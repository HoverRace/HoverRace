
// Button.h
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

#pragma once

#include "Res.h"

#include "ClickRegion.h"
#include "FillBox.h"

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
		class BaseContainer;
		class Display;
		class Label;
		class Picture;
		class Texture;
	}
}

namespace HoverRace {
namespace Display {

/**
 * A standard button, with a text label.
 * @author Michael Imamura
 */
class MR_DllDeclare Button : public ClickRegion
{
	using SUPER = ClickRegion;

public:
	struct Props
	{
		enum
		{
			TEXT = SUPER::Props::NEXT_,
			ICON,
			IMAGE,
			CONTENTS,
			NEXT_,  ///< First index for subclasses.
		};
	};

public:
	Button(Display &display, const std::string &text,
		uiLayoutFlags_t layoutFlags = 0);
	Button(Display &display, const Vec2 &size, const std::string &text,
		uiLayoutFlags_t layoutFlags = 0);
	virtual ~Button();

private:
	void Init(const std::string &text);

public:
	void AttachView(Display &disp) override { AttachViewDynamic(disp, this); }

public:
	bool OnNavigate(const Control::Nav &nav) override;

public:
	const std::string &GetText() const;
	void SetText(const std::string &text);

	Alignment getTextAlignment() const;
	void SetTextAlignment(Alignment textAlignment);

	Box *GetIcon() const { return icon.get(); }
	std::shared_ptr<Box> ShareIcon() const { return icon; }
	void SetIcon(std::shared_ptr<Box> icon);

	std::shared_ptr<Res<Texture>> ShareTexture() const;
	void SetTexture(std::shared_ptr<Res<Texture>> image);

	void SetContents(std::shared_ptr<BaseContainer> contents);

	Box *GetBackgroundChild() const { return background.get(); }
	Label *GetLabelChild() const { return label.get(); }
	Box *GetIconChild() const { return icon.get(); }
	BaseContainer *GetContentsChild() const { return contents.get(); }
	Picture *GetPictureChild() const { return picture ? picture.get() : nullptr; }
	FillBox *GetHighlightChild() const { return highlight.get(); }

	void SetFixedWidth(double width);

protected:
	void Layout() override;

public:
	Vec3 Measure() override;
	void FireModelUpdate(int prop) override;

private:
	std::unique_ptr<FillBox> background;
	std::unique_ptr<Label> label;
	std::shared_ptr<Box> icon;
	std::shared_ptr<BaseContainer> contents;
	std::unique_ptr<Picture> picture;
	std::unique_ptr<FillBox> highlight;
	boost::optional<double> fixedWidth;
	double paddingTop, paddingRight, paddingBottom, paddingLeft, iconGap;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
