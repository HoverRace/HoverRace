
// Emitter.h
//
// Copyright (c) 2008, 2009, 2015-2016 Michael Imamura.
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

#include <exception>
#include <string>

#include <yaml.h>

#include "../OS.h"
#include "YamlExn.h"

namespace HoverRace {
namespace Util {
namespace yaml {

/// Standard exception thrown for emitter errors.
class EmitterExn : public YamlExn
{
	using SUPER = YamlExn;

public:
	EmitterExn() : SUPER() { }
	EmitterExn(const char *const &msg) : SUPER(msg)  { }
	virtual ~EmitterExn() noexcept { }
};

/// Wrapper for the LibYAML emitter.
class Emitter
{
private:
	Emitter() = delete;
public:
	Emitter(FILE *file, bool versionDirective = true);
	Emitter(std::ostream &os, bool versionDirective = true);
	Emitter(std::string &s, bool versionDirective = true);
	virtual ~Emitter();

protected:
	void InitEmitter();
	void InitStream(bool versionDirective);

public:
	void StartMap();
	void MapKey(const std::string &s);
	void EndMap();

	void StartSeq();
	void EndSeq();

	void Value(const std::string &val);
	void Value(const char *val);
	void Value(bool val);
	void Value(int val);
	void Value(double val);
	void Value(float val);
	void Value(const HoverRace::Util::OS::path_t &path);

	template<class T>
	void Value(T&& val)
	{
		Value(boost::lexical_cast<std::string>(val));
	}

private:
	yaml_emitter_t emitter;
};

}  // namespace yaml
}  // namespace Util
}  // namespace HoverRace
