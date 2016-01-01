#!/usr/bin/env bash

# doxyclean
#   Performs a "clean" run of Doxygen.
#   Stale documentation is removed, and optionally the changes are
#   prepared for committing.

cd "$(dirname "$0")"/.. || exit 1

DOXYGEN="${DOXYGEN:-doxygen}"

# Check the Doxygen version.
REQ_DOXYGEN_VER='1.8.11'
DOXYGEN_VER="$("$DOXYGEN" --version)"
if [[ $DOXYGEN_VER != "$REQ_DOXYGEN_VER" ]]; then
	echo "Required Doxygen version: $REQ_DOXYGEN_VER" >&2
	echo 'Set DOXYGEN to the full path to the Doxygen binary.' >&2
	exit 1
fi

# Clean up previous files.
# We just remove the outputs to preserve any source control files that
# are in the directory.
if [[ -d srcdoc/html ]]; then
	(
		cd srcdoc/html || exit 1
		rm -f -- \
			*.css \
			*.html \
			*.js \
			*.map \
			*.md5 \
			*.png \
			*.svg
	)
fi

# Run Doxygen twice.
# On some versions of Doxygen, the formatting of the headers and footers
# change on the second run and beyond.  We thus always run it twice to
# minimize the number of changed lines.
"$DOXYGEN" && "$DOXYGEN" || exit 1

# If the documentation is linked to a Git repository, then prepare the
# files for commit, including removing deleted files.
if [[ -d srcdoc/html/.git ]]; then
	pushd srcdoc/html || exit 1
	git add . || exit 1
	git add -u || exit 1
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

