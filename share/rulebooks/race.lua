
game:new_rulebook {
	name = 'Race',
	description = 'First player to complete the number of laps wins',
	
	on_pre_game = function(session)
		print('on_pre_game in rulebook')
	end,

	on_post_game = function(session)
		print('on_session_end in rulebook')
	end,
}
