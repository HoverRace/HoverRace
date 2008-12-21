
lib_LTLIBRARIES = libhoverrace-engine.la
libhoverrace_engine_la_SOURCES = \
	Util/FuzzyLogic.cpp

AM_CXXFLAGS = -Winvalid-pch

BUILT_SOURCES = StdAfx.h.gch
CLEANFILES = $(BUILT_SOURCES)

# Build the precompiled header.
# It may not be correct to explicitly set -fPIC, but it's required
# otherwise GCC will complain that the PCH was built with different
# options.
StdAfx.h.gch: StdAfx.h
	$(CXXCOMPILE) -x c++-header -fPIC -DPIC -c $<

