
// Label.h
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

#pragma once

#include "../VideoServices/FontSpec.h"
#include "Color.h"

#include "ViewModel.h"

#ifdef _WIN32
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
	}
}

namespace HoverRace {
namespace Display {

/**
 * A static text label.
 * This is intended for text elements that rarely change over time, aside from
 * visibility.
 * @author Michael Imamura
 */
class MR_DllDeclare Label : public ViewModel
{
	typedef ViewModel SUPER;

	public:
		struct Props
		{
			enum {
				COLOR,
				FONT,
				TEXT,
			};
		};

	public:
		Label(const std::string &text, const VideoServices::FontSpec &font,
			const Color color);
		virtual ~Label();

	public:
		virtual void AttachView(Display *disp) { AttachViewDynamic(disp, this); }

	public:
		const Color GetColor() const { return color; }
		void SetColor(const Color color);

		const VideoServices::FontSpec &GetFont() const { return font; }
		void SetFont(VideoServices::FontSpec &font);

		const std::string &GetText() const { return text; }
		void SetText(const std::string &text);
#		ifdef _WIN32
			const std::wstring &GetWText() const { return wtext; }
#		endif

	private:
		std::string text;
#		ifdef _WIN32
			/// Cached wide string since that's how it's used in Win32.
			std::wstring wtext;
#		endif
		VideoServices::FontSpec font;
		Color color;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
