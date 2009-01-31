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

# Extract the "prerelease" flag.
FLAGS=`grep '^ FILEFLAGS 0x[0-9]*L' "$INFILE" | head -1 | grep -o '0x[0-9]*'`
fltest=$(( FLAGS & 2 ))
if [ $fltest -gt 0 ]; then
	PRERELEASE=true
else
	PRERELEASE=false
fi

# Generate version.h,
cat <<EOD > "$OUTFILE"
#pragma once
#define HR_APP_VERSION $VER
#define HR_APP_VERSION_PRERELEASE $PRERELEASE
EOD

# Echo it out so it can be captured when generating configure.
echo -n "$ver"

