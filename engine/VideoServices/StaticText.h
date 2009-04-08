
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

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

#include <string>

#include "../Util/MR_Types.h"
#include "../VideoServices/Font.h"

class MR_2DViewPort;

namespace HoverRace {
namespace VideoServices {

/**
 * An i18n-capable text renderable.
 * @author Michael Imamura
 */
class MR_DllDeclare StaticText {
	public:
		StaticText(const std::string &s="",
			const std::string &font="Arial",
			int size=20, bool bold=false, bool italic=false,
			MR_UInt8 color=43);
		StaticText(const std::string &s,
			const HoverRace::VideoServices::Font &font,
			MR_UInt8 color=43);
		virtual ~StaticText();

		int GetWidth() const;
		int GetHeight() const;

		void SetText(const std::string &s);
		void SetFont(const HoverRace::VideoServices::Font &font);
		void SetColor(MR_UInt8 color);

	private:
		void Update();

	public:
		void Blt(int x, int y, MR_2DViewPort *pDest, bool centerX=false) const;

	private:
		std::string s;
		wchar_t *ws;
		size_t wsLen;

		HoverRace::VideoServices::Font font;
		MR_UInt8 color;

		MR_UInt8 *bitmap;
		int width;
		int height;
};

}  // namespace VideoServices
}  // namespace HoverRace
