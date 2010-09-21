
ACLOCAL_FLAGS = -I config/m4
AUTOMAKE_OPTIONS = foreign no-dist-gzip dist-bzip2

EXTRA_DIST = m4/ChangeLog  config/config.rpath  \
	build \
	hovercad \
	imr \
	include \
	lib \
	res \
	share \
	Doxyfile \
	credits.txt \
	hoverrace.sln \
	license.txt

SUBDIRS = engine client compilers po

pkgdata_DATA = \
	res/Intro.avi \
	res/ObjFac1.dat

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck


ACLOCAL_AMFLAGS = -I m4
