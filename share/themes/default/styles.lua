
--[[
	This Lua script is the stylesheet for the default theme.
	
	Note that all of the styles are commented out.
	This is because we're actually using the built-in defaults.
	
	Think of this as a guide to what the original values are when making your
	own themes.
]]--

-- This is a workaround for a bug when rendering in software mode.
local WHITE = 0xfffefefe

--[[
styles.body_font:set { size = 30, style = Font.PLAIN }
styles.body_fg = 0xffbfbfbf
styles.body_head_font:set { size = 30, style = Font.PLAIN }
styles.body_head_fg = WHITE
styles.body_aside_font:set { size = 20, style = Font.PLAIN }
styles.body_aside_fg = 0xffbfbfbf

styles.announcement_head_font:set { size = 30, style = Font.PLAIN }
styles.announcement_head_fg = WHITE
styles.announcement_body_font:set { size = 20, style = Font.PLAIN }
styles.announcement_body_fg = 0xff7f7f7f
styles.announcement_symbol_fg = 0xbfffffff
styles.announcement_bg = 0xbf000000

-- 100% HUD scale.
styles.hud_normal_font:set { size = 30, style = Font.BOLD }
styles.hud_normal_head_font:set { size = 20, style = Font.PLAIN }
-- 50% HUD scale.
styles.hud_small_font:set { size = 20, style = Font.BOLD }
styles.hud_small_head_font:set { size = 15, style = Font.PLAIN }

styles.console_font:set { name = "monospace", size = 30, style = Font.PLAIN }
styles.console_fg = WHITE
styles.console_cursor_fg = 0xffbfbfbf
styles.console_bg = 0xbf000000

styles.form_font = styles.body_font
styles.form_fg = WHITE
styles.form_disabled_fg = 0x7fffffff

styles.dialog_bg = 0xcc000000

styles.grid_margin.x = 6
styles.grid_margin.y = 6
styles.grid_padding.x = 1
styles.grid_padding.y = 1

styles.button_bg = 0x3f00007f
styles.button_disabled_bg = 0x3f7f7f7f
styles.button_pressed_bg = 0x7f00007f

styles.heading_font:set { size = 40, style = Font.BOLD }
styles.heading_fg = WHITE
]]--
