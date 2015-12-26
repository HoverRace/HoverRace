
// UiFont.h
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

#pragma once

#include "../Util/Hash.h"
#include "../Util/SelFmt.h"

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
 * Describes a font used for the UI.
 * @author Michael Imamura
 */
struct MR_DllDeclare UiFont
{
	UiFont(const std::string &name, double size = 20.0, int style = 0) :
		name(name), size(size), style(style) { }
	UiFont(double size = 20.0, int style = 0) :
		name(), size(size), style(style) { }

	UiFont(const UiFont &o) = default;
	UiFont(UiFont &&o) = default;

	UiFont &operator=(const UiFont &o) = default;
	UiFont &operator=(UiFont &&o) = default;

	void Set(const std::string &name, double size = 20.0, int style = 0)
	{
		this->name = name;
		this->size = size;
		this->style = style;
	}

	void Set(double size = 20.0, int style = 0) noexcept
	{
		this->name.clear();
		this->size = size;
		this->style = style;
	}

	bool isBold() const noexcept { return !!(style & BOLD); }
	bool isItalic() const noexcept { return !!(style & ITALIC); }

	enum Style {
		BOLD = 0x01,
		ITALIC = 0x02,
		BOLD_ITALIC = BOLD | ITALIC,
	};

	std::string name;
	double size;
	int style;
};

MR_DllDeclare inline bool operator==(const UiFont &a, const UiFont &b)
{
	return
		static_cast<int>(a.size) == static_cast<int>(b.size) &&
		a.style == b.style &&
		a.name == b.name;
}

MR_DllDeclare inline bool operator!=(const UiFont &a, const UiFont &b)
{
	return !operator==(a, b);
}

MR_DllDeclare inline std::ostream &operator<<(std::ostream &os,
	const UiFont &fs)
{
	if (fs.name.empty()) {
		os << "(default)";
	}
	else {
		os << fs.name;
	}
	if (fs.isBold()) os << " Bold";
	if (fs.isItalic()) os << " Italic";
	os << ' ' << fs.size;

	return os;
}

}  // namespace Display
}  // namespace HoverRace

namespace std {

template<>
struct hash<HoverRace::Display::UiFont>
{
	std::size_t operator()(const HoverRace::Display::UiFont &font) const
	{
		using namespace HoverRace::Util;

		std::size_t seed = 0;
		Hash::Combine(seed, font.name);
		Hash::Combine(seed, static_cast<int>(font.size));
		Hash::Combine(seed, font.style);
		return seed;
	}
};

}  // namespace std

#undef MR_DllDeclare
