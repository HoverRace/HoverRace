
return Session {

	on_init = function(self)
		-- Constructor-ish initialization.
		print("on_init in Session")
	end,

	on_pregame = function(self)
		local session = self.session
		local num_players = session:get_num_players()

		session:countdown_to_next_phase(
			num_players == 1 and "6s" or "20s")

		print("on_pregame in Session")
		print("laps = " .. session.rules.laps)
		print("num players = " .. num_players)
	end,

	-- Called when the *first* player calls finish().
	on_postgame = function(self)
		print("on_postgame in Session")
		-- Remaining players have 30 seconds to finish racing.
		--[[ TODO: Session phases
		self.session:countdown_to_next_phase("30s")
		]]--
	end,

	-- Called when all players have called finish() or the timer ran out.
	on_done = function(self)
		--[[
		local results = Results(self.session.players)

		--TODO: Support teams?
		for player in results:each_player() do
			results.add_rank {
				total_time = player.finish_time,
				best_lap = player.best_lap,
			}
		end

		results.add_meta {
			total_time = self.session.clock
		}

		self.session:post_results(results)
		]]--
	end,
}
