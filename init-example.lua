
-- init-example.lua

-- Print a message so we know that this script is being executed.
print("Executing init script.")

-- This script is executed after the config has been read but before
-- the main window has been created.
-- This gives us a chance to modify the config if we want.
do
	local cfg = game:get_config()
	cfg:unlink()  -- Prevent our modifications from being saved.
	local w, h = cfg:get_video_res()
	print("Configured window res: " .. w .. "x" .. h)
	cfg:set_video_res(800, 450)
	w, h = cfg:get_video_res()
	print("New window res: " .. w .. "x" .. h)
end

-- Map some useful hotkeys.
-- Note that F11 is the default key to bring up the console
-- and F12 is used to break into the debugger in some IDEs.
input:hotkey("f1", function()
	game:toggle_hud()
end)
input:hotkey("f2", function()
	game:screenshot()
end)
input:hotkey("f10", function ()
	debug:start_test_lab()
end)

-- Some functions, such as game:start_practice(), can't be called until
-- the game has been initialized (when the on_init handlers are fired).
print("Initialized: ", game:is_initialized())

-- on_init handlers are called just after the main window is visible
-- and all subsystems have been started up.
game:on_init(function()
	print("In on_init handler")
	print("Initialized: ", game:is_initialized())
	-- The session won't actually start until all on_init handlers
	-- have finished executing.
	game:start_practice("ClassicH", {
		laps = 2
		})
end)

-- We can attach as many handlers to each event as we want!
game:on_init(function()
	print("In second on_init handler")
end)
game:on_init(function()
	print("In third on_init handler")
end)

local function second_session(session)
	-- Remove the "session_start" on_session_start handler.
	game:on_session_begin("session_start", nil)
	print("Starting up the second session")
end

local function first_session(session)
	-- Replace the "session_start" on_session_start handler
	-- so the next session will fire second_session instead
	-- of this one.
	game:on_session_begin("session_start", second_session)
	print("Starting up the first session.  Num players:",
		session:get_num_players())

	--[[ Some ideas for expanding the API...
	local clock = session:get_clock()
	-- Can also say clock:from_now("3s")
	clock:at(clock:from_now(3000), function()
		-- Start the next session
		game:start_practice("The Alley2")
	end)
	--]]
end

game:on_session_begin("session_begin", first_session)

game:on_session_end(function(session)
	print("Ending a session")
end)

game:on_shutdown(function()
	print("In on_shutdown handler.")
end)
