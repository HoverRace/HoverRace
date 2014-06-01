
return Player {

	on_init = function(self)
		print("In overloaded on_init!")
	
		-- Constructor-ish initialization.
		self.stopwatch = nil
		self.finish_time = nil
	end,

	on_joined = function(self, session)
		self.session = session
		self.lap = 1
	end,
	
	on_start = function(self)
		local session = self.session.session
		local player_name = self.player.name;

		-- Set up the stopwatch to time each lap.
		self.stopwatch = Stopwatch(session.clock)

		-- Set up the racing HUD.
		-- We start with the default HUD for a race and add our lap counter.
		local hud = self.player.hud
		hud:use_race_default()
		hud:add_chronometer(Hud.S, "Time", session.clock)
		self.lap_counter = hud:add_counter(Hud.S, "Lap", 1,
			session.rules.laps)
		print(player_name .. " started at " .. tostring(session.clock))
	end,
	
	on_finish_line = function(self)
		local session = self.session.session
		local player_name = self.player.name
		
		local lap = self.lap
		local lap_time = self.stopwatch:next_lap("Lap " .. lap)
		lap = lap + 1
		self.lap = lap

		--TODO: Track best lap.

		print(player_name .. " lap time: " .. tostring(lap_time))
		print(player_name .. " is on lap " ..
			lap .. "/" .. session.rules.laps ..
			" (" .. (session.time / 1000) .. ")")

		self.lap_counter.value = lap
		if lap > session.rules.laps then
			print(player_name .. " finished race!")
			self.player:finish()
		end
	end,
	
	on_finish = function(self)
		local session = self.session.session
		self.finish_time = session.clock

		local hud = self.player.hud
		--TODO: Set up postgame HUD.
		hud:clear()
		hud:add_text(Hud.S, "Finished in " .. tostring(self.finish_time))
		print(self.player.name .. " finished at " .. tostring(self.finish_time))
	end,

}
