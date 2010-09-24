
AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = hoverrace
hoverrace_CPPFLAGS = $(HR_CPPFLAGS) -DLOCALEDIR=\"$(localedir)\"
hoverrace_CXXFLAGS = $(HR_CXXFLAGS)
hoverrace_LDADD = \
	../../engine/libhoverrace-engine.la \
	$(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_SYSTEM_LDFLAGS) $(BOOST_SYSTEM_LIBS) \
	$(BOOST_THREAD_LDFLAGS) $(BOOST_THREAD_LIBS) \
	$(LUA_LIBS) \
	$(DEPS_LIBS)
hoverrace_SOURCES = \
	StdAfx.h \
	Control/SDL/SDLInputManager.cpp \
	Control/SDL/SDLInputManager.h \
	Control/SDL/SDLKeyboard.cpp \
	Control/SDL/SDLKeyboard.h \
	Control/SDL/SDLMouse.cpp \
	Control/SDL/SDLMouse.h \
	Control/SDL/SDLPrereqs.h \
	Control/Controller.cpp \
	Control/Controller.h \
	Control/InputHandler.h \
	Control/UiHandler.h \
	HoverScript/ClientScriptCore.cpp \
	HoverScript/ClientScriptCore.h \
	HoverScript/ConfigPeer.cpp \
	HoverScript/ConfigPeer.h \
	HoverScript/Console.cpp \
	HoverScript/Console.h \
	HoverScript/GamePeer.cpp \
	HoverScript/GamePeer.h \
	HoverScript/HighConsole.cpp \
	HoverScript/HighConsole.h \
	HoverScript/SessionPeer.cpp \
	HoverScript/SessionPeer.h \
	HoverScript/SysEnv.cpp \
	HoverScript/SysEnv.h \
	ClientApp.cpp \
	ClientApp.h \
	ClientSession.cpp \
	ClientSession.h \
	GameDirector.h \
	RoomList.cpp \
	RoomList.h \
	Rulebook.h \
	main.cpp \
	version.h

BUILT_SOURCES = StdAfx.h.gch
CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = \
	Game2.rc \
	StdAfx.cpp \
	icon1.ico \
	icon2.ico \
	icon3.ico

# Build the precompiled header.
StdAfx.h.gch: StdAfx.h
	$(CXXCOMPILE) $(HR_CPPFLAGS) $(HR_CXXFLAGS) -x c++-header -c -o $@ $<

