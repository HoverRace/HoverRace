
// KeycapIcon.h
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

/**
 * Icon for a keycap (keyboard key or gamepad button(.
 * @author Michael Imamura
 */
class MR_DllDeclare KeycapIcon : public FillBox
{
	using SUPER = FillBox;

public:
	struct Props
	{
		enum
		{
			KEY_HASH = SUPER::Props::NEXT_,
			NEXT_,  ///< First index for subclasses.
		};
	};

public:
	KeycapIcon(double height, int keyHash,
		const Color color = COLOR_WHITE, uiLayoutFlags_t layoutFlags = 0);
	virtual ~KeycapIcon() { }

public:
	void AttachView(Display &disp) override { AttachViewDynamic(disp, this); }

public:
	int GetKeyHash() const { return keyHash; }
	void SetKeyHash(int keyHash);

	void AdjustHeight(double h) override;
	void AdjustWidth(double w) override;
	
public:
	Vec3 Measure() override
	{
		return GetView() ? GetView()->Measure() : Vec3(0, 0, 0);
	}

private:
	int keyHash;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
