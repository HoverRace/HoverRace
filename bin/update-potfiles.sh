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

function filter_modern {
	while read -r fn; do
		# Ignore legacy source files.
		! grep -q '#ifdef HR_LEGACY_BUILD' "$fn" && echo "$fn"
	done
}

for srcdir in "${SRCDIRS[@]}"; do
	find "$srcdir" -type f -name '*.cpp' \
		-not -name 'StdAfx.*' \
		-not -name '.*'
done | sort | filter_modern

