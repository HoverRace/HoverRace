#!/bin/sh

cd `dirname $0`/../share

rm -rf locale
mkdir locale
for lingua in "$@"; do
	lingua=`basename "$lingua" .gmo`
	gmofile="../po/$lingua.gmo"
	if [ -f "$gmofile" ]; then
		echo "$lingua"
		mkdir "locale/$lingua"
		mkdir "locale/$lingua/LC_MESSAGES"
		ln -s \
			"../../../$gmofile" \
			"locale/$lingua/LC_MESSAGES/hoverrace.mo"
	fi
done

