#!/usr/bin/env bash

cd "$(dirname "$0")/../share" || exit 1

rm -rf locale
mkdir locale
for lingua in "$@"; do
	lingua="$(basename "$lingua" .gmo)"
	gmofile="../po/$lingua.gmo"
	if [ -f "$gmofile" ]; then
		echo -n "$lingua "
		mkdir "locale/$lingua"
		mkdir "locale/$lingua/LC_MESSAGES"
		ln -s \
			"../../../$gmofile" \
			"locale/$lingua/LC_MESSAGES/hoverrace.mo"
	fi
done
echo

