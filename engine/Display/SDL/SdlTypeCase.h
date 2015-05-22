
// SdlTypeCase.h
//
// Copyright (c) 2015 Michael Imamura.
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

#include "../TypeCase.h"

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
		namespace SDL {
			class SdlDisplay;
			class SdlDynamicTexture;
		}
	}
}

namespace HoverRace {
namespace Display {
namespace SDL {

/**
 * SDL implementation of TypeCase.
 * @author Michael Imamura
 */
class MR_DllDeclare SdlTypeCase :
	public TypeCase,
	public std::enable_shared_from_this<SdlTypeCase>
{
	using SUPER = TypeCase;

public:
	SdlTypeCase(SdlDisplay &display, const UiFont &font, int width, int height);
	SdlTypeCase(const SdlTypeCase&) = delete;
	virtual ~SdlTypeCase();

	SdlTypeCase &operator=(const SdlTypeCase&) = delete;

public:
	MR_UInt32 CountTextures() const override;

private:
	static int MeasureSpaceWidth(SdlDisplay &display, const UiFont &font);
	GlyphEntry &AddGlyph(GlyphEntry &ent, const std::string &s,
		MR_UInt32 cp, std::string &added);
	GlyphEntry &FindGlyph(const std::string &s, MR_UInt32 cp,
		std::string &added);

public:
	void Prepare(const std::string &s, TypeLine *rects = nullptr) override;
	void Render(const TypeLine &s, const Color color, int x, int y) override;
	void RenderTexture(MR_UInt32 idx, int x, int y,
		double scale = 1.0) override;

protected:
	SdlDisplay &display;
private:
	int fontHeight;
	int spaceWidth;
	using glyphPage_t = std::array<GlyphEntry, 256>;
	std::array<std::unique_ptr<glyphPage_t>, 256> glyphs;
	std::vector<std::unique_ptr<SdlDynamicTexture>> maps;
	MR_UInt32 curMap;
	int curX, curY;
};

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
