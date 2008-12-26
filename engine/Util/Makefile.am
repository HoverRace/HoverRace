
SUBDIRS = yaml

noinst_LTLIBRARIES = libutil.la
libutil_la_CPPFLAGS = -I..
libutil_la_LIBADD = yaml/libutil-yaml.la
libutil_la_SOURCES = \
	Config.cpp \
	Config.h \
	FastArray.h \
	FastFifo.h \
	FuzzyLogic.cpp \
	FuzzyLogic.h \
	MR_Types.h \
	Pixel.h

AM_CXXFLAGS = -Winvalid-pch $(BOOST_CPPFLAGS) $(DEPS_CFLAGS)

