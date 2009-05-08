
// StaticText.h
// Header for the StaticText renderable.
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

#pragma once

#include <string>

#include "../Util/MR_Types.h"
#include "../VideoServices/Font.h"

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

class MR_2DViewPort;

namespace HoverRace {
namespace VideoServices {

/**
 * An i18n-capable text renderable.
 * @author Michael Imamura
 */
class MR_DllDeclare StaticText {
	public:
		enum effect_t {
			EFFECT_NONE,
			EFFECT_SHADOW,
		};

		StaticText(const std::string &s="",
			const std::string &font="Arial",
			int size=20, bool bold=false, bool italic=false,
			MR_UInt8 color=43, effect_t effect=EFFECT_NONE);
		StaticText(const std::string &s,
			const HoverRace::VideoServices::Font &font,
			MR_UInt8 color=43, effect_t effect=EFFECT_NONE);
		virtual ~StaticText();

		int GetWidth() const;
		int GetHeight() const;
		int GetRealWidth() const;
		int GetRealHeight() const;

		void SetText(const std::string &s);
		void SetFont(const HoverRace::VideoServices::Font &font);
		void SetColor(MR_UInt8 color);

	private:
		void Update();

		void ApplyShadow();

	public:
		void Blt(int x, int y, MR_2DViewPort *pDest, bool centerX=false) const;

	private:
		std::string s;
		wchar_t *ws;
		size_t wsLen;

		HoverRace::VideoServices::Font font;
		MR_UInt8 color;
		effect_t effect;

		MR_UInt8 *bitmap;
		int width;
		int height;
		int realWidth;
		int realHeight;
};

}  // namespace VideoServices
}  // namespace HoverRace

#undef MR_DllDeclare
