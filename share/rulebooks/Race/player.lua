
return Player {

	on_init = function(self)
		print("Overridden on_init call!")

		-- Constructor-ish initialization.
		self.stopwatch = nil
		self.finish_time = nil
	end,

}
