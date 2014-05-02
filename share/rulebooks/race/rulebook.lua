
Rulebook "Race" {
	title = "Race",
	description = "First player to finish all laps wins",

	rules = {
		laps = 5,
	},

	on_pre_game = function(session)
		print("on_pre_game in rulebook")
		print("laps = " .. session.rules.laps)
		print("num players = " .. session:get_num_players())
	end,

	on_player_joined = function(session, player)
		local player_name = player.name

		-- Set up the player properties and events.
		player.props.lap = 1
		player:on_start(function()
			-- Set up the stopwatch to time each lap.
			player.props.stopwatch = Stopwatch(session.clock)

			-- Set up the racing HUD.
			-- We start with the default HUD for a race and add our lap counter.
			local hud = player.hud
			hud:use_race_default()
			hud:add_chronometer(Hud.S, "Time", session.clock)
			player.props.lap_counter = hud:add_counter(Hud.S, "Lap", 1,
				session.rules.laps)
			print(player_name .. " started at " .. tostring(session.clock))
		end)
		player:on_finish_line(function()
			local lap = player.props.lap
			local lap_time = player.props.stopwatch:next_lap("Lap " .. lap)
			lap = lap + 1
			player.props.lap = lap

			print(player_name .. " lap time: " .. tostring(lap_time))
			print(player_name .. " is on lap " ..
				lap .. "/" .. session.rules.laps ..
				" (" .. (session.time / 1000) .. ")")

			player.props.lap_counter.value = lap
			if lap > session.rules.laps then
				print(player_name .. " finished race!")
				player:finish()
			end
		end)
		player:on_finish(function()
			local hud = player.hud
			--TODO: Set up postgame HUD.
			hud:clear()
			hud:add_text(Hud.S, "Finished in " .. tostring(session.clock))
			print(player_name .. " finished at " .. tostring(session.clock))
		end)

		--TODO: Set up the pregame HUD for the player.
	end,

	on_post_game = function(session)
		print("on_session_end in rulebook")
	end,
}
