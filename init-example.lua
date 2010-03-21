
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
	print("Configured window res: ", w, h)
	cfg:set_video_res(800, 600)
	w, h = cfg:get_video_res()
	print("New window res: ", w, h)
end

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
	game:start_practice("ClassicH.trk", {
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

function second_session(session)
	-- Remove the "session_start" on_session_start handler.
	game:on_session_start("session_start", nil)
	print("Starting up the second session")
end

function first_session(session)
	-- Replace the "session_start" on_session_start handler
	-- so the next session will fire second_session instead
	-- of this one.
	game:on_session_start("session_start", second_session)
	print("Starting up the first session.  Num players:",
		session:get_num_players())
	
	--[[ Some ideas for expanding the API...
	local player = session:get_local_player()
	-- local player = session:get_player(1)
	local track = session:get_track()
	print("Track name: ", track:get_name())
	local clock = session:get_clock()
	-- Can also say clock:from_now("3s")
	clock:at(clock:from_now(3000), function()
		-- Start the next session
		game:start_practice("ClasicH.trk")
	end)
	--]]
end

game:on_session_start("session_start", first_session)
