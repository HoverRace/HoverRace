#!/bin/sh

INFILE=client/Game2/Game2.rc
OUTFILE=client/Game2/version.h

VER=`grep '^ FILEVERSION [0-9,]*' "$INFILE" | grep -o '[0-9,]*'`

MAJOR=`echo "$VER" | sed 's/^\([0-9]*\),\([0-9]*\),\([0-9]*\),\([0-9]*\)$/\1/'`
MINOR=`echo "$VER" | sed 's/^\([0-9]*\),\([0-9]*\),\([0-9]*\),\([0-9]*\)$/\2/'`
PATCH=`echo "$VER" | sed 's/^\([0-9]*\),\([0-9]*\),\([0-9]*\),\([0-9]*\)$/\3/'`
BUILD=`echo "$VER" | sed 's/^\([0-9]*\),\([0-9]*\),\([0-9]*\),\([0-9]*\)$/\4/'`

# Render the short form of the version.
ver="$MAJOR.$MINOR"
if [ "$PATCH" != '0' ]; then
	ver="$ver.$PATCH"
	if [ "$BUILD" != '0' ]; then
		ver="$ver.$BUILD"
	fi
fi

# Generate version.h,
cat <<EOD > "$OUTFILE"
#pragma once
#define HR_APP_VERSION $VER
EOD

# Echo it out so it can be captured when generating configure.
echo -n "$ver"

