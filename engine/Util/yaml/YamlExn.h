
// YamlExn.h
//
// Copyright (c) 2008, 2009, 2015 Michael Imamura.
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
namespace Util {
namespace yaml {

class YamlExn : public std::exception
{
	using SUPER = std::exception;

public:
	YamlExn() : SUPER() { }
	YamlExn(const char* const &msg) : SUPER(), msg(msg) { }
	YamlExn(const std::string &msg) : SUPER(), msg(msg) { }
	virtual ~YamlExn() noexcept { }

	const char* what() const noexcept override { return msg.c_str(); }

private:
	std::string msg;
};

}  // namespace yaml
}  // namespace Util
}  // namespace HoverRace

