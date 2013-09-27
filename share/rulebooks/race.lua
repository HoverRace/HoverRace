
local function init_player(session, player, idx)
	local player_name = 'Player ' .. (idx + 1)

	player.props.lap = 1
	player:on_finish_line(function()
		local lap = player.props.lap + 1
		player.props.lap = lap

		print(player_name .. ' is on lap ' .. lap)
		if lap == session.rules.lap then
			print(player_name .. ' finished race!')
			player:finish()
		end
	end)
end

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
		for i = 0, #session.players do
			init_player(session, session.players[i], i)
		end
	end,

	on_post_game = function(session)
		print('on_session_end in rulebook')
	end,
}
