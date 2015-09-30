#!/usr/bin/env bash

# update-potfiles.sh
#   Generates a list of files which might contain translatable strings.
#
# Usage:
#   bin/update-potfiles.sh > po/potfiles.txt

cd "$(dirname "$0")"/.. || exit 1

SRCDIRS=(
	client/Game2
	compilers/MazeCompiler
	compilers/ResourceCompiler
	engine
	)

for srcdir in "${SRCDIRS[@]}"; do
	find "$srcdir" -type f -name '*.cpp' \
		-not -name 'StdAfx.*' \
		-not -name '.*'
done | sort

