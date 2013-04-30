
bin_PROGRAMS = hr-parceldump
hr_parceldump_CPPFLAGS = $(HR_CPPFLAGS)
hr_parceldump_CXXFLAGS = $(HR_CXXFLAGS)
hr_parceldump_LDADD = \
	../../engine/libhoverrace-engine.la \
	$(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_SYSTEM_LDFLAGS) $(BOOST_SYSTEM_LIBS) \
	$(BOOST_THREADS_LDFLAGS) $(BOOST_THREADS_LIBS) \
	$(LUA_LIBS) \
	$(DEPS_LIBS) $(TEXT_DEPS_LIBS)
hr_parceldump_SOURCES = \
	StdAfx.h \
	main.cpp

BUILT_SOURCES = StdAfx.h.gch
CLEANFILES = $(BUILT_SOURCES) StdAfx.h.Td StdAfx.h.d

EXTRA_DIST = \
	ParcelDump.vcxproj \
	ParcelDump.vcxproj.filters \
	StdAfx.cpp

# Build the precompiled header.
StdAfx.h.gch: StdAfx.h
	$(AM_V_GEN)$(CXXCOMPILE) $(HR_CPPFLAGS) $(HR_CXXFLAGS) \
		-MD -MP -MF StdAfx.h.Td \
		-x c++-header -c -o $@ $<
	mv StdAfx.h.Td StdAfx.h.d

-include StdAfx.h.d

