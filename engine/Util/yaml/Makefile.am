
noinst_LTLIBRARIES = libutil-yaml.la
libutil_yaml_la_CPPFLAGS = -I../..
libutil_yaml_la_LDFLAGS = -lyaml
libutil_yaml_la_SOURCES = \
	Emitter.cpp \
	Emitter.h \
	MapNode.cpp \
	MapNode.h \
	Node.h \
	NodeFactory.cpp \
	NodeFactory.h \
	Parser.cpp \
	Parser.h \
	ScalarNode.cpp \
	ScalarNode.h \
	SeqNode.cpp \
	SeqNode.h \
	YamlExn.h

AM_CXXFLAGS = -Winvalid-pch $(BOOST_CPPFLAGS) $(DEPS_CFLAGS)

