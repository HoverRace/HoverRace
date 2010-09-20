
AUTOMAKE_OPTIONS = subdir-objects

noinst_LTLIBRARIES = libutil.la
libutil_la_CPPFLAGS = -I.. $(HR_CPPFLAGS)
libutil_la_CXXFLAGS = $(HR_CXXFLAGS)
libutil_la_LDFLAGS = $(YAML_LDFLAGS)
libutil_la_LIBADD = $(LTLIBICONV)
libutil_la_SOURCES = \
	yaml/Emitter.cpp \
	yaml/Emitter.h \
	yaml/MapNode.cpp \
	yaml/MapNode.h \
	yaml/Node.h \
	yaml/NodeFactory.cpp \
	yaml/NodeFactory.h \
	yaml/Parser.cpp \
	yaml/Parser.h \
	yaml/ScalarNode.cpp \
	yaml/ScalarNode.h \
	yaml/SeqNode.cpp \
	yaml/SeqNode.h \
	yaml/YamlExn.h \
	BitPacking.h \
	Config.cpp \
	Config.h \
	DllObjectFactory.cpp \
	DllObjectFactory.h \
	FastArray.h \
	FastFifo.h \
	FuzzyLogic.cpp \
	FuzzyLogic.h \
	InspectMapNode.cpp \
	InspectMapNode.h \
	InspectNode.h \
	Inspectable.h \
	MR_Types.h \
	OS.cpp \
	OS.h \
	Str.cpp \
	Str.h \
	WorldCoordinates.cpp \
	WorldCoordinates.h

