
Rulebook 'Race' {
	title = 'Race',
	description = 'First player to complete the number of laps wins',
	
	rules = {
		laps = 5,
	},
	
	on_pre_game = function(session)
		print('on_pre_game in rulebook')
		print('laps = ' .. session.rules.laps)
		session.players[0]:on_finish_line(function()
			print('Player 1 has crossed the finish line')
		end)
	end,

	on_post_game = function(session)
		print('on_session_end in rulebook')
	end,
}
