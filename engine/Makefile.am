
BUILT_SOURCES = StdAfx.h.gch

SUBDIRS = ColorTools Util

lib_LTLIBRARIES = libhoverrace-engine.la
libhoverrace_engine_la_CPPFLAGS = $(HR_CPPFLAGS)
libhoverrace_engine_la_CXXFLAGS = $(HR_CXXFLAGS)
libhoverrace_engine_la_LIBADD = \
	ColorTools/libcolortools.la \
	Util/libutil.la
libhoverrace_engine_la_LDFLAGS = \
	$(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_FILESYSTEM_LIBS) \
	$(DEPS_LIBS)
libhoverrace_engine_la_SOURCES = \
	StdAfx.cpp \
	StdAfx.h

CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = \
	engine.vcproj

# Build the precompiled header.
# It may not be correct to explicitly set -fPIC, but it's required
# otherwise GCC will complain that the PCH was built with different
# options.
StdAfx.h.gch: StdAfx.h
	$(CXXCOMPILE) $(HR_CPPFLAGS) $(HR_CXXFLAGS) -x c++-header -fPIC -DPIC -c -o $@ $<

