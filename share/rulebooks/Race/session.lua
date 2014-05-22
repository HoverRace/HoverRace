
return Session {

	on_init = function(self)
		-- Constructor-ish initialization.
		print("on_init in Session")
	end,

	on_pregame = function(self)
		local session = self.session

		--[[ TODO: Session phases
		session:countdown_to_next_phase("10s")
		]]--
		print("on_pregame in Session")
		print("laps = " .. session.rules.laps)
		print("num players = " .. session:get_num_players())
	end,

	-- Called when the *first* player calls finish().
	on_postgame = function(self)
		print("on_postgame in Session")
		-- Remaining players have 30 seconds to finish racing.
		--[[ TODO: Session phases
		self.session:countdown_to_next_phase("30s")
		]]--
	end,

}
