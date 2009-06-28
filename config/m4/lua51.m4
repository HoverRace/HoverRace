
dnl lua51.m4
dnl   Autoconf macros for Lua 5.1.
dnl 
dnl Copyright (C) 2008 Michael Imamura <zoogie@lugatgt.org>
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

AC_DEFUN([LUA51_REQUIRE],
[
PKG_CHECK_EXISTS([lua >= 5.1], [
	PKG_CHECK_MODULES(Lua, [lua >= 5.1], [lua_found=yes])
], [
	PKG_CHECK_MODULES(Lua, [lua5.1 >= 5.1], [lua_found=yes], [lua_found=no])
])
if test x"$lua_found" = xyes
then
	LUA_CFLAGS=Lua_CFLAGS
	LUA_LIBS=Lua_LIBS
else
	LUA_CFLAGS=
	LUA_LIBS=
fi
AC_SUBST([LUA_CFLAGS])
AC_SUBST([LUA_LIBS])
])

