#!/bin/sh

# autogen-debug.sh
#   This is a convenience script to set common build settings used for
#   development (debug) builds.
#
#   Alternatively, you can specify a prefix to autogen.sh and set up a
#   config.site file.  See for details:
#   http://www.gnu.org/software/automake/manual/html_node/config_002esite.html

SELF=`basename "$0"`
SELFDIR=`dirname "$SELF"`

# Set common settings used for debug builds.
CPPFLAGS="$CPPFLAGS -D_DEBUG"; export CPPFLAGS
CFLAGS="$CFLAGS -g -O0"; export CFLAGS
CXXFLAGS="$CXXFLAGS -g -O0"; export CXXFLAGS

"$SELFDIR/autogen.sh" "$@"
