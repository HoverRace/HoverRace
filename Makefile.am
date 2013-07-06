
ACLOCAL_AMFLAGS = -I m4 --install ${ACLOCAL_FLAGS}
AUTOMAKE_OPTIONS = foreign no-dist-gzip dist-bzip2

EXTRA_DIST = config/config.rpath  \
	build \
	hovercad \
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
	share/Intro.avi \
	share/ObjFac1.dat

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

