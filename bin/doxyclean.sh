#!/usr/bin/env bash

# doxyclean
#   Performs a "clean" run of Doxygen.
#   Stale documentation is removed, and optionally the changes are
#   prepared for committing.

set -e

cd "$(dirname "$0")"/..

# Clean up previous files.
# We just remove the outputs to preserve any source control files that
# are in the directory.
if [[ -d srcdoc/html ]]; then
	pushd srcdoc/html
	rm -f -- \
		*.css \
		*.html \
		*.js \
		*.map \
		*.md5 \
		*.png \
		*.svg
	popd
fi

# Run Doxygen twice.
# On some versions of Doxygen, the formatting of the headers and footers
# change on the second run and beyond.  We thus always run it twice to
# minimize the number of changed lines.
doxygen && doxygen

# If the documentation is linked to a Git repository, then prepare the
# files for commit, including removing deleted files.
if [[ -d srcdoc/html/.git ]]; then
	pushd srcdoc/html
	git add .
	git add -u
	popd

	# We don't automatically commit.
	# This is the point where we can check the warnings and correct them
	# before publishing.
	echo '--> Ready to commit!  Verify the output, then to publish use:'
	echo 'cd srcdoc/html ; git commit -m "Update source documentation from Doxygen."'

	if [[ -s srcdoc/warnings.txt ]]; then
		echo
		echo '--> Doxygen warnings detected!'
		cat srcdoc/warnings.txt
	fi
fi

