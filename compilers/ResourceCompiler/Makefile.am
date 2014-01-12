
bin_PROGRAMS = hr-resourcecompiler
hr_resourcecompiler_CPPFLAGS = $(HR_CPPFLAGS) -DLOCALEDIR=\"$(localedir)\"
hr_resourcecompiler_CXXFLAGS = $(HR_CXXFLAGS)
hr_resourcecompiler_LDADD = \
	../../engine/libhoverrace-engine.la \
	$(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_SYSTEM_LDFLAGS) $(BOOST_SYSTEM_LIBS) \
	$(BOOST_THREADS_LDFLAGS) $(BOOST_THREADS_LIBS) \
	$(LUA_LIBS) \
	$(DEPS_LIBS) $(TEXT_DEPS_LIBS) $(AUDIO_DEPS_LIBS)
hr_resourcecompiler_SOURCES = \
	StdAfx.h \
	BitmapHelper.cpp \
	BitmapHelper.h \
	ResActorBuilder.cpp \
	ResActorBuilder.h \
	ResBitmapBuilder.cpp \
	ResBitmapBuilder.h \
	ResSoundBuilder.cpp \
	ResSoundBuilder.h \
	ResSpriteBuilder.cpp \
	ResSpriteBuilder.h \
	ResourceCompiler.cpp \
	ResourceLibBuilder.cpp \
	ResourceLibBuilder.h \
	TextParsing.cpp \
	TextParsing.h

BUILT_SOURCES = StdAfx.h.gch
CLEANFILES = $(BUILT_SOURCES) StdAfx.h.Td StdAfx.h.d

EXTRA_DIST = \
	ResourceCompiler.vcxproj \
	ResourceCompiler.vcxproj.filters \
	StdAfx.cpp

# Build the precompiled header.
StdAfx.h.gch: StdAfx.h
	$(AM_V_GEN)$(CXXCOMPILE) $(HR_CPPFLAGS) $(HR_CXXFLAGS) \
		-MD -MP -MF StdAfx.h.Td \
		-x c++-header -c -o $@ $<
	mv StdAfx.h.Td StdAfx.h.d

-include StdAfx.h.d

