
SUBDIRS = yaml

noinst_LTLIBRARIES = libutil.la
libutil_la_CPPFLAGS = -I.. $(HR_CPPFLAGS)
libutil_la_CXXFLAGS = $(HR_CXXFLAGS)
libutil_la_LIBADD = yaml/libutil-yaml.la
libutil_la_SOURCES = \
	Config.cpp \
	Config.h \
	FastArray.h \
	FastFifo.h \
	FuzzyLogic.cpp \
	FuzzyLogic.h \
	MR_Types.h \
	Pixel.h \
	WorldCoordinates.cpp \
	WorldCoordinates.h

