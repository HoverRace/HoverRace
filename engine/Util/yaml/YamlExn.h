
/* yaml/YamlExn.h
	Header for yaml::YamlExn. */

#pragma once

namespace yaml
{
	class YamlExn : public std::exception
	{
		typedef std::exception SUPER;

		public:
			YamlExn() : SUPER() { }
			YamlExn(const char* const &msg) : SUPER(), msg(msg) { }
			YamlExn(const std::string &msg) : SUPER(), msg(msg) { }
			virtual ~YamlExn() throw() { }

			virtual const char* what() const throw() { return msg.c_str(); }

		private:
			std::string msg;
	};
}

