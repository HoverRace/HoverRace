
Rulebook 'Race' {
	title = 'Race',
	description = 'First player to complete the number of laps wins',
	
	rules = {
		laps = 5,
	},
	
	on_pre_game = function(session)
		print('on_pre_game in rulebook')
	end,

	on_post_game = function(session)
		print('on_session_end in rulebook')
	end,
}
