#!/bin/sh

SELF=`basename "$0"`

MKFILE=build/mkversion.sh
RCFILE=client/Game2/Game2.rc
VERFILE=client/Game2/version.h

# Check if we need to invalidate the autom4te cache.
# This is so that mkversion.sh will be re-run as necessary.
if [ -e autom4te.cache -a \( \
	\! -f "$VERFILE" -o \
	"$0" -nt "$VERFILE" -o \
	"$RCFILE" -nt "$VERFILE" -o \
	"$MKFILE" -nt "$VERFILE" \
	\) ]
then
	echo "$SELF: Clearing autom4te cache"
	rm -rf autom4te.cache
fi

autoreconf -v --install || exit 1
./configure "$@"
