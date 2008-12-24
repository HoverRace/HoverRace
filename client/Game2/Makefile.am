
bin_PROGRAMS = hoverrace
hoverrace_SOURCES = \
	StdAfx.h \
	main-unix.cpp \
	version.h

AM_CXXFLAGS = -Winvalid-pch $(BOOST_CPPFLAGS)

BUILT_SOURCES = StdAfx.h.gch
CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = \
	Game2.exe.manifest \
	Game2.rc \
	StdAfx.cpp \
	icon1.ico \
	icon2.ico \
	icon3.ico

# Build the precompiled header.
StdAfx.h.gch: StdAfx.h
	$(CXXCOMPILE) -x c++-header -c -o $@ $<

