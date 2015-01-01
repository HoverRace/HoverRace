
// StyleEnv.cpp
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

#include "../../StdAfx.h"

#include "../../../engine/Display/Display.h"
#include "../../../engine/Util/Log.h"
#include "../../../engine/Util/Str.h"

#include "StyleEnv.h"

using namespace HoverRace::Script;
using namespace HoverRace::Util;
namespace fs = boost::filesystem;

namespace luabind {

// Treat Color in Lua as a simple number.

template<>
struct default_converter<HoverRace::Display::Color> :
	native_converter_base<HoverRace::Display::Color>
{
	static int compute_score(lua_State* L, int index)
	{
		return lua_type(L, index) == LUA_TNUMBER ? 0 : -1;
	}

	HoverRace::Display::Color from(lua_State* L, int index)
	{
		return static_cast<MR_UInt32>(lua_tonumber(L, index));
	}

	void to(lua_State* L, HoverRace::Display::Color const& x)
	{
		lua_pushnumber(L, x.argb);
	}
};

template<>
struct default_converter<HoverRace::Display::Color const&> :
	default_converter<HoverRace::Display::Color>
{ };

}  // namespace luabind

namespace HoverRace {
namespace Client {
namespace HoverScript {

/**
 * Constructor.
 *
 * The base path is the directory of the theme that will be loaded.
 *
 * @param scripting The scripting context.
 * @param display The display to which the styles will be applied.
 * @param basePath The root directory of the theme.
 */
StyleEnv::StyleEnv(Script::Core *scripting, Display::Display &display,
	const Util::OS::path_t &basePath) :
	SUPER(scripting),
	display(display), basePath(basePath)
{
}

/**
 * Register bindings in an environment.
 * @param scripting The target environment.
 */
void StyleEnv::Register(Script::Core *scripting)
{
	using namespace luabind;
	using Styles = Display::Display::styles_t;
	lua_State *L = scripting->GetState();

	// Register base classes.
	module(L) [
		class_<Styles, Styles*>("Styles")
			.def_readwrite("body_font", &Styles::bodyFont)
			.def_readwrite("body_fg", &Styles::bodyFg)
			.def_readwrite("body_head_font", &Styles::bodyHeadFont)
			.def_readwrite("body_head_fg", &Styles::bodyHeadFg)
			.def_readwrite("body_aside_font", &Styles::bodyAsideFont)
			.def_readwrite("body_aside_fg", &Styles::bodyAsideFg)
			.def_readwrite("announcement_head_font", &Styles::announcementHeadFont)
			.def_readwrite("announcement_head_fg", &Styles::announcementHeadFg)
			.def_readwrite("announcement_body_font", &Styles::announcementBodyFont)
			.def_readwrite("announcement_body_fg", &Styles::announcementBodyFg)
			.def_readwrite("announcement_symbol_fg", &Styles::announcementSymbolFg)
			.def_readwrite("announcement_bg", &Styles::announcementBg)
			.def_readwrite("hud_normal_font", &Styles::hudNormalFont)
			.def_readwrite("hud_normal_head_font", &Styles::hudNormalHeadFont)
			.def_readwrite("hud_small_font", &Styles::hudSmallFont)
			.def_readwrite("hud_small_head_font", &Styles::hudSmallHeadFont)
			.def_readwrite("console_font", &Styles::consoleFont)
			.def_readwrite("console_fg", &Styles::consoleFg)
			.def_readwrite("console_cursor_fg", &Styles::consoleCursorFg)
			.def_readwrite("console_bg", &Styles::consoleBg)
			.def_readwrite("form_font", &Styles::formFont)
			.def_readwrite("form_fg", &Styles::formFg)
			.def_readwrite("form_disabled_fg", &Styles::formDisabledFg)
			.def_readwrite("dialog_bg", &Styles::dialogBg)
			.def_readwrite("grid_margin", &Styles::gridMargin)
			.def_readwrite("grid_padding", &Styles::gridPadding)
			.def_readwrite("button_bg", &Styles::buttonBg)
			.def_readwrite("button_disabled_bg", &Styles::buttonDisabledBg)
			.def_readwrite("button_pressed_bg", &Styles::buttonPressedBg)
			.def_readwrite("heading_font", &Styles::headingFont)
			.def_readwrite("heading_fg", &Styles::headingFg)
	];
}

void StyleEnv::InitEnv()
{
	using namespace luabind;

	SUPER::InitEnv();

	lua_State *L = GetScripting()->GetState();

	// Start with the standard global environment.
	CopyGlobals();

	object env(from_stack(L, -1));
	env["styles"] = &display.styles;
}

/**
 * Run the stylesheet.
 */
bool StyleEnv::RunStylesheet()
{
	auto stylePath = basePath;
	stylePath /= Str::UP("styles.lua");

	if (!fs::exists(stylePath)) {
		HR_LOG(info) << "Stylesheet path does not have a styles.lua: " <<
			basePath;
		return false;
	}

	HR_LOG(info) << "Running: " << stylePath;

	return RunScript(stylePath);
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
