
bin_PROGRAMS = hoverrace
hoverrace_CPPFLAGS = $(HR_CPPFLAGS) -DLOCALEDIR=\"$(localedir)\"
hoverrace_CXXFLAGS = $(HR_CXXFLAGS)
hoverrace_LDADD = \
	../../engine/libhoverrace-engine.la \
	$(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_SYSTEM_LDFLAGS) $(BOOST_SYSTEM_LIBS) \
	$(BOOST_THREADS_LDFLAGS) $(BOOST_THREADS_LIBS) \
	$(LUA_LIBS) \
	$(DEPS_LIBS)
hoverrace_SOURCES = \
	StdAfx.h \
	ClientScriptCore.cpp \
	ClientScriptCore.h \
	ConfigPeer.cpp \
	ConfigPeer.h \
	Console.cpp \
	Console.h \
	Controller.cpp \
	Controller.h \
	GamePeer.cpp \
	GamePeer.h \
	RoomList.cpp \
	RoomList.h \
	Rulebook.h \
	SessionPeer.cpp \
	SessionPeer.h \
	SysEnv.cpp \
	SysEnv.h \
	main.cpp \
	version.h

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
	$(CXXCOMPILE) $(HR_CPPFLAGS) $(HR_CXXFLAGS) -x c++-header -c -o $@ $<

