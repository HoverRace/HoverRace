
BUILT_SOURCES = StdAfx.h.gch

SUBDIRS = ColorTools Util

lib_LTLIBRARIES = libhoverrace-engine.la
libhoverrace_engine_la_LIBADD = \
	ColorTools/libcolortools.la \
	Util/libutil.la
libhoverrace_engine_la_LDFLAGS = \
	$(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_FILESYSTEM_LIBS) \
	$(DEPS_LIBS)
libhoverrace_engine_la_SOURCES = \
	StdAfx.cpp \
	StdAfx.h

AM_CXXFLAGS = -Winvalid-pch $(BOOST_CPPFLAGS) $(DEPS_CFLAGS)

CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = \
	engine.vcproj

# Build the precompiled header.
# It may not be correct to explicitly set -fPIC, but it's required
# otherwise GCC will complain that the PCH was built with different
# options.
StdAfx.h.gch: StdAfx.h
	$(CXXCOMPILE) -x c++-header -fPIC -DPIC -c -o $@ $<

