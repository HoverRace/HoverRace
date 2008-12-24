
/* yaml/Emitter.h
	Header for yaml::Emitter. */

#pragma once

#include <exception>
#include <string>

#include <yaml.h>

#include "YamlExn.h"

namespace yaml
{
	/// Standard exception thrown for emitter errors.
	class EmitterExn : public YamlExn
	{
		typedef YamlExn SUPER;

		public:
			EmitterExn() : SUPER() { }
			EmitterExn(const char *const &msg) : SUPER(msg)  { }
			virtual ~EmitterExn() throw() { }
	};

	/// Wrapper for the LibYAML emitter.
	class Emitter
	{
		private:
			Emitter() { }
		public:
			Emitter(FILE *file);
			virtual ~Emitter();

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

		private:
			yaml_emitter_t emitter;
	};
}
