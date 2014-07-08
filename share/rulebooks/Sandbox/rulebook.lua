
Rulebook "Sandbox" {
	title = "Sandbox",
	description = "No rules, no time limit, just explore",

	on_load = function()
		return {
			session = require("session"),
		}
	end,
}
