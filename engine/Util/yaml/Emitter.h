
/* yaml/Emitter.h
	Header for yaml::Emitter. */

#pragma once

#include <exception>
#include <string>

#include <yaml.h>

namespace yaml
{
	/// Standard exception thrown for emitter errors.
	class EmitterExn : public std::exception
	{
		typedef std::exception SUPER;

		public:
			EmitterExn() : SUPER() { }
			EmitterExn(const char *const &msg) : SUPER(), msg(msg)  { }
			virtual ~EmitterExn() throw() { }

			virtual const char* what() const throw() { return msg.c_str(); }

		private:
			std::string msg;
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
