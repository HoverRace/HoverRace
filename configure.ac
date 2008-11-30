AC_PREREQ(2.59)
AC_INIT(HoverRace,1.24,[http://svn.igglybob.com/hoverrace/])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR(config/m4)
AC_CONFIG_SRCDIR([NetTarget/Util/Config.cpp])
AM_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE

AC_LIBTOOL_DLOPEN

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

# Checks for libraries.
AC_LIB_LTDL
BOOST_REQUIRE(1.34)
BOOST_FILESYSTEM
BOOST_THREADS

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.


AC_CONFIG_FILES([
    Makefile
    NetTarget/Makefile
    NetTarget/Util/Makefile
])
AC_OUTPUT
