
lib_LTLIBRARIES = libhoverrace-engine.la
libhoverrace_engine_la_SOURCES = \
	ColorTools/ColorTools.cpp \
	ColorTools/ColorTools.h \
	Util/Config.cpp \
	Util/Config.h \
	Util/FuzzyLogic.cpp \
	Util/FuzzyLogic.h \
	Util/MR_Types.h \
	Util/yaml/Emitter.cpp \
	Util/yaml/Emitter.h \
	Util/yaml/MapNode.cpp \
	Util/yaml/MapNode.h \
	Util/yaml/Node.h \
	Util/yaml/NodeFactory.cpp \
	Util/yaml/NodeFactory.h \
	Util/yaml/Parser.cpp \
	Util/yaml/Parser.h \
	Util/yaml/ScalarNode.cpp \
	Util/yaml/ScalarNode.h \
	Util/yaml/SeqNode.cpp \
	Util/yaml/SeqNode.h \
	Util/yaml/YamlExn.h \
	StdAfx.h

AM_CXXFLAGS = -Winvalid-pch $(BOOST_CPPFLAGS)

BUILT_SOURCES = StdAfx.h.gch
CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = \
	StdAfx.cpp \
	engine.vcproj

# Build the precompiled header.
# It may not be correct to explicitly set -fPIC, but it's required
# otherwise GCC will complain that the PCH was built with different
# options.
StdAfx.h.gch: StdAfx.h
	$(CXXCOMPILE) -x c++-header -fPIC -DPIC -c -o $@ $<

