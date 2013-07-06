
AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = hoverrace
hoverrace_CPPFLAGS = $(HR_CPPFLAGS) -DLOCALEDIR=\"$(localedir)\"
hoverrace_CXXFLAGS = $(HR_CXXFLAGS)
hoverrace_LDADD = \
	../../engine/libhoverrace-engine.la \
	$(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_SYSTEM_LDFLAGS) $(BOOST_SYSTEM_LIBS) \
	$(BOOST_THREAD_LDFLAGS) $(BOOST_THREAD_LIBS) \
	$(LUA_LIBS) $(LUABIND_LDFLAGS) \
	$(DEPS_LIBS) $(TEXT_DEPS_LIBS)
hoverrace_SOURCES = \
	StdAfx.h \
	HoverScript/ClientScriptCore.cpp \
	HoverScript/ClientScriptCore.h \
	HoverScript/ConfigPeer.cpp \
	HoverScript/ConfigPeer.h \
	HoverScript/Console.cpp \
	HoverScript/Console.h \
	HoverScript/ConsoleScene.cpp \
	HoverScript/ConsoleScene.h \
	HoverScript/DebugPeer.cpp \
	HoverScript/DebugPeer.h \
	HoverScript/GamePeer.cpp \
	HoverScript/GamePeer.h \
	HoverScript/SessionPeer.cpp \
	HoverScript/SessionPeer.h \
	HoverScript/SysConsole.cpp \
	HoverScript/SysConsole.h \
	HoverScript/SysEnv.cpp \
	HoverScript/SysEnv.h \
	ClientApp.cpp \
	ClientApp.h \
	ClientSession.cpp \
	ClientSession.h \
	FolderViewer.cpp \
	FolderViewer.h \
	FormScene.cpp \
	FormScene.h \
	GameDirector.h \
	GameScene.cpp \
	GameScene.h \
	MainMenuScene.cpp \
	MainMenuScene.h \
	MessageScene.cpp \
	MessageScene.h \
	Observer.cpp \
	Observer.h \
	PaletteScene.cpp \
	PaletteScene.h \
	RoomList.cpp \
	RoomList.h \
	Rulebook.h \
	Scene.h \
	Scene.cpp \
	TestLabScene.h \
	TestLabScene.cpp \
	UiScene.h \
	UiScene.cpp \
	main.cpp \
	version.h

BUILT_SOURCES = StdAfx.h.gch
CLEANFILES = $(BUILT_SOURCES) StdAfx.h.Td StdAfx.h.d

EXTRA_DIST = \
	Game2.rc \
	StdAfx.cpp \
	icon1.ico \
	icon2.ico \
	icon3.ico

# Build the precompiled header.
StdAfx.h.gch: StdAfx.h
	$(AM_V_GEN)$(CXXCOMPILE) $(HR_CPPFLAGS) $(HR_CXXFLAGS) \
		-x c++-header \
		-MD -MP -MF StdAfx.h.Td \
		-c -o $@ $<
	mv StdAfx.h.Td StdAfx.h.d

-include StdAfx.h.d

