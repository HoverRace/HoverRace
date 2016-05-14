
// RulebookLibrary.h
//
// Copyright (c) 2013-2016 Michael Imamura.
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
	}
	namespace Script {
		class Core;
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
	RulebookLibrary(Script::Core &scripting);

public:
	void Reload();
	void Add(std::shared_ptr<Rulebook> &rulebook);
	std::shared_ptr<const Rulebook> GetDefault() const;
	std::shared_ptr<const Rulebook> Find(const std::string &name);

private:
	using sorted_t = std::set<
		std::shared_ptr<const Rulebook>,
		boost::less_pointees_t<std::shared_ptr<const Rulebook>>>;
public:
	using const_iterator = sorted_t::const_iterator;
	using value_type = sorted_t::value_type;

	const_iterator begin() const { return sorted.cbegin(); }
	const_iterator end() const { return sorted.cend(); }
	const_iterator cbegin() const { return sorted.cbegin(); }
	const_iterator cend() const { return sorted.cend(); }

private:
	Script::Core &scripting;
	std::map<std::string, std::shared_ptr<Rulebook>> library;
	sorted_t sorted;
	std::shared_ptr<Rulebook> defaultRulebook;
};

}  // namespace Client
}  // namespace HoverRace
