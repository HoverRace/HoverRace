
SUBDIRS = yaml

noinst_LTLIBRARIES = libutil.la
libutil_la_CPPFLAGS = -I.. $(HR_CPPFLAGS)
libutil_la_CXXFLAGS = $(HR_CXXFLAGS)
libutil_la_LIBADD = $(LTLIBICONV) yaml/libutil-yaml.la
libutil_la_SOURCES = \
	BitPacking.h \
	Config.cpp \
	Config.h \
	FastArray.h \
	FastFifo.h \
	FuzzyLogic.cpp \
	FuzzyLogic.h \
	InspectMapNode.cpp \
	InspectMapNode.h \
	InspectNode.h \
	Inspectable.h \
	MR_Types.h \
	OS.cpp \
	OS.h \
	Str.cpp \
	Str.h \
	WorldCoordinates.cpp \
	WorldCoordinates.h

