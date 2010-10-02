
BUILT_SOURCES = StdAfx.h.gch

SUBDIRS = \
	ColorTools \
	MainCharacter \
	Model \
	Net \
	ObjFac1 \
	ObjFacTools \
	Parcel \
	Script \
	Util \
	VideoServices

lib_LTLIBRARIES = libhoverrace-engine.la
libhoverrace_engine_la_CPPFLAGS = $(HR_CPPFLAGS)
libhoverrace_engine_la_CXXFLAGS = $(HR_CXXFLAGS)
libhoverrace_engine_la_LIBADD = \
	ColorTools/libcolortools.la \
	MainCharacter/libmainchar.la \
	Model/libmodel.la \
	Net/libnet.la \
	ObjFac1/libobjfac1.la \
	ObjFacTools/libobjfactools.la \
	Parcel/libparcel.la \
	Script/libscript.la \
	Util/libutil.la \
	VideoServices/libvideosvc.la
libhoverrace_engine_la_LDFLAGS = \
	$(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_FILESYSTEM_LIBS) \
	$(LUABIND_LDFLAGS) \
	$(DEPS_LIBS)
libhoverrace_engine_la_SOURCES = \
	StdAfx.h \
	Exception.h
EXTRA_libhoverrace_engine_la_SOURCES = \
	StdAfx.cpp

CLEANFILES = $(BUILT_SOURCES) StdAfx.h.Td StdAfx.h.d

EXTRA_DIST = \
	engine.vcxproj \
	engine.vcxproj.filters

# Build the precompiled header.
# It may not be correct to explicitly set -fPIC, but it's required
# otherwise GCC will complain that the PCH was built with different
# options.
StdAfx.h.gch: StdAfx.h
	$(CXXCOMPILE) $(HR_CPPFLAGS) $(HR_CXXFLAGS) -x c++-header -fPIC -DPIC \
		-MD -MP -MF StdAfx.h.Td \
		-c -o $@ $<
	mv StdAfx.h.Td StdAfx.h.d

-include StdAfx.h.d

