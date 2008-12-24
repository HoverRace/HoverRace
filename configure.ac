AC_PREREQ(2.59)
AC_INIT(
	HoverRace,
	[esyscmd(build/mkversion.sh)],
	[http://svn.igglybob.com/hoverrace/])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR(config/m4)
AC_CONFIG_SRCDIR([engine/Util/FuzzyLogic.h])
AM_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

# Checks for libraries.
BOOST_REQUIRE(1.34)
BOOST_FILESYSTEM
BOOST_THREADS
PKG_CHECK_MODULES(DEPS, [
	libcurl >= 7.18
])
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.


AC_CONFIG_FILES([
	Makefile
	client/Makefile
	client/Game2/Makefile
	engine/Makefile
])
AC_OUTPUT
