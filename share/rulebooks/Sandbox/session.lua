
return Session {

	on_pregame = function(self)
		-- Set a short countdown for now.
		-- Once we can properly handle switching out of pregame immediately,
		-- then we can remove this.
		self.session:countdown_to_next_phase("1s")
	end,

}
