
// RulebookLibrary.h
//
// Copyright (c) 2013 Michael Imamura.
//
// Licensed under GrokkSoft HoverRace SourceCode License v1.0(the "License");
// you may not use this file except in compliance with the License.
//
// A copy of the license should have been attached to the package from which
// you have taken this file. If you can not find the license you can not use
// this file.
//
//
// The author makes no representations about the suitability of
// this software for any purpose.  It is provided "as is" "AS IS",
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.
//
// See the License for the specific language governing permissions
// and limitations under the License.

#pragma once

namespace HoverRace {
	namespace Client {
		class Rulebook;
		typedef std::shared_ptr<Rulebook> RulebookPtr;
	}
}

namespace HoverRace {
namespace Client {

/**
 * The collection of all registered rulebooks.
 * @author Michael Imamura
 */
class RulebookLibrary {
	public:
		RulebookLibrary();

	public:
		void Add(RulebookPtr &rulebook);
		RulebookPtr Find(const std::string &name);

	private:
		typedef std::set<RulebookPtr, boost::less_pointees_t<RulebookPtr>> sorted_t;
	public:
		typedef sorted_t::const_iterator const_iterator;
		typedef sorted_t::value_type value_type;

		const_iterator begin() const { return sorted.cbegin(); }
		const_iterator end() const { return sorted.cend(); }
		const_iterator cbegin() const { return sorted.cbegin(); }
		const_iterator cend() const { return sorted.cend(); }

	private:
		typedef std::map<std::string, RulebookPtr> library_t;
		library_t library;
		sorted_t sorted;
};

}  // namespace Client
}  // namespace HoverRace
