#!/usr/bin/env bash

# cppcheck
#   Runs cppcheck, a static code analyzer, against all C++ sources.
#   This assumes that cppcheck is available in the path.
#   http://cppcheck.sourceforge.net/
#
# After checking, the report of warnings is available in: cppcheck_report.txt

CPPCHECK=cppcheck

cd "$(dirname "$0")/.."

( find client/Game2 -name '*.cpp' ; find engine -name '*.cpp' ) | \
	"$CPPCHECK" --file-list=- 2> >(tee cppcheck_report.txt >&2)

