
Rulebook 'Race' {
	title = 'Race',
	description = 'First player to complete the number of laps wins',

	rules = {
		laps = 5,
	},

	on_pre_game = function(session)
		print('on_pre_game in rulebook')
		print('laps = ' .. session.rules.laps)
		print('num players = ' .. #session.players)
	end,

	on_player_init = function(session, player)
		local player_name = 'Player ' .. (player.index + 1)

		player.props.lap = 1
		player:on_finish_line(function()
			local lap = player.props.lap + 1
			player.props.lap = lap

			print(player_name .. ' is on lap ' ..
				lap .. '/' .. session.rules.laps)
			if lap > session.rules.laps then
				print(player_name .. ' finished race!')
				player:finish()
			end
		end)
	end,

	on_post_game = function(session)
		print('on_session_end in rulebook')
	end,
}
