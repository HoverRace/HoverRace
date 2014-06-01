
Rulebook "Race" {
	title = "Race",
	description = "First player to finish all laps wins",

	rules = {
		laps = 5,
	},

	on_load = function()
		return {
			player = require("player"),
			session = require("session"),
		}
	end,

}
