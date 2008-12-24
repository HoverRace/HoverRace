
lib_LTLIBRARIES = libhoverrace-engine.la
libhoverrace_engine_la_SOURCES = \
	ColorTools/ColorTools.cpp \
	ColorTools/ColorTools.h \
	Util/FuzzyLogic.cpp \
	Util/FuzzyLogic.h \
	Util/MR_Types.h \
	StdAfx.h

AM_CXXFLAGS = -Winvalid-pch

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

