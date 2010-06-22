
dnl luabind.m4
dnl   Autoconf macros for Luabind.
dnl 
dnl Copyright (C) 2010 Michael Imamura <zoogie@lugatgt.org>
dnl All rights reserved.
dnl 
dnl Redistribution and use in source and binary forms, with or
dnl without modification, are permitted provided that the following
dnl conditions are met:
dnl
dnl * Redistributions of source code must retain the above copyright
dnl   notice, this list of conditions and the following disclaimer.
dnl * Redistributions in binary form must reproduce the above copyright
dnl   notice, this list of conditions and the following disclaimer in
dnl   the documentation and/or other materials provided with the
dnl   distribution.
dnl * Neither the name of the author nor the names of its contributors
dnl   may be used to endorse or promote products derived from this
dnl   software without specific prior written permission.
dnl
dnl THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
dnl "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
dnl LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
dnl FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
dnl COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
dnl INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
dnl BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
dnl OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
dnl AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
dnl LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
dnl ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
dnl POSSIBILITY OF SUCH DAMAGE.

dnl LUABIND_REQUIRE()
dnl Check for Luabind, with support for alternate install locations
dnl via "--with-luabind=".
dnl If found, LUABIND_CPPFLAGS and LUABIND_LDFLAGS will be set.
AC_DEFUN([LUABIND_REQUIRE],
[
dnl Required Luabind version.
dnl TODO: Make this configurable.
luabind_ver_major=0
luabind_ver_minor=7
luabind_ver_patch=0
luabind_ver_str="$luabind_ver_major.$luabind_ver_minor.$luabind_ver_patch"
AC_ARG_WITH([luabind],
	AC_HELP_STRING([--with-luabind=DIR],
		[prefix for Luabind library and headers]),
	[
		dnl We need to make sure that distcheck uses the same setting
		dnl otherwise it will fail.
		AC_SUBST([DISTCHECK_CONFIGURE_FLAGS],
			["$DISTCHECK_CONFIGURE_FLAGS '--with-luabind=$with_luabind'"])
	])
# Search for Luabind.
AC_CACHE_CHECK([for Luabind version >= $luabind_ver_str], [luabind_cv_path],
	[
		AC_LANG_PUSH([C++])
		luabind_cv_path=no
		for luabind_dir in "$with_luabind" '' /usr/local /usr
		do
			luabind_save_CPPFLAGS="$CPPFLAGS"
			luabind_save_LDFLAGS="$LDFLAGS"
			luabind_save_LIBS="$LIBS"
			if test x"$luabind_dir" != x
			then
				test -e "$luabind_dir/include/luabind/luabind.hpp" || continue
				CPPFLAGS="$CPPFLAGS -I$luabind_dir/include"
				_LUABIND_DETECT_LIB([$luabind_dir], [luabind_dir_lib])
				test x"$luabind_dir_lib" = x && continue
				LDFLAGS="$LDFLAGS -L$luabind_dir_lib -Wl,-rpath,$luabind_dir_lib"
			fi
			dnl We assume LUA51_REQUIRE has been run.
			CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS $LUA_CFLAGS"
			LIBS="$LIBS $LUA_LIBS -lluabind"
			AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#include <luabind/luabind.hpp>
#include <luabind/version.hpp>
				]],[[
int ver_major = LUABIND_VERSION / 10000;
int ver_minor = LUABIND_VERSION / 100 % 100;
int ver_patch = LUABIND_VERSION % 100;
if (ver_major < $luabind_ver_major) return 1;
if (ver_minor < $luabind_ver_minor &&
	ver_major == $luabind_ver_major) return 1;
if (ver_patch < $luabind_ver_patch &&
	ver_minor == $luabind_ver_minor &&
	ver_major == $luabind_ver_major) return 1;
				]])],
				[luabind_cv_path=yes],
				[luabind_cv_path=no])
			CPPFLAGS="$luabind_save_CPPFLAGS"
			LDFLAGS="$luabind_save_LDFLAGS"
			LIBS="$luabind_save_LIBS"
			if test x"$luabind_cv_path" = xyes
			then
				if test x"$luabind_dir" != x
				then
					luabind_cv_path="$luabind_dir"
				fi
				break
			fi
		done
		AC_LANG_POP([C++])
	])
if test x"$luabind_cv_path" = xno
then
	AC_MSG_ERROR([Could not find Luabind version >= $luabind_ver_str; try using --with-luabind=/path/to/luabind])
elif test x"$luabind_cv_path" = xyes
then
	LUABIND_CPPFLAGS=
	LUABIND_LDFLAGS="-lluabind"
else
	LUABIND_CPPFLAGS="-I$luabind_cv_path/include"
	_LUABIND_DETECT_LIB([$luabind_cv_path], [luabind_dir_lib])
	if test x"$luabind_dir_lib" = x
	then
		AC_MSG_ERROR([Internal error: Could not determine Luabind library directory])
	else
		LUABIND_LDFLAGS="-L$luabind_dir_lib -lluabind"
	fi
fi
AC_SUBST([LUABIND_CPPFLAGS])
AC_SUBST([LUABIND_LDFLAGS])
])

dnl _LUABIND_DETECT_LIB(prefix, varname)
dnl Attempt to determine the name of the library directory for a prefix.
dnl Result is stored in vername.  If no known library path exists, then
dnl varname will be set to the empty string.
AC_DEFUN([_LUABIND_DETECT_LIB],
[
	$2=
	for luabind_detect_dir in "$1/lib64" "$1/lib" "$1/lib32"
	do
		if test -d "$luabind_detect_dir"
		then
			$2="$luabind_detect_dir"
			break
		fi
	done
])

