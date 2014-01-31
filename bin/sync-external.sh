#!/bin/bash

# sync-external.sh
#   Get latest updates for a 3rd-party library in external.
#
# Usage:
#   bin/sync-external.sh libs...
#
# Example:
#   bin/sync-external.sh lua luabind
#
# Since git-subtree synthesizes new commits to the repository, you should
# run this in a new branch, make sure everything got synchronized properly,
# then merge into master.

# git-subtree must be run from the root of the repo.
cd "$(dirname "$0")"/..

if [[ $1 = '' ]]; then
	echo 'Usage:' >&2
	echo "  $0 libs..." >&2
	echo >&2
	echo 'To update all external libraries at once (be careful!):' >&2
	echo "  $0 $(cd external ; find . -mindepth 1 -maxdepth 1 -type d -exec basename \{\} \; | sort | xargs echo)"
	exit 1
fi

# Check for git-subtree.
if ! git help -a | grep -q '  subtree'; then
	echo 'git-subtree is not installed; see:' >&2
	echo 'https://github.com/git/git/tree/master/contrib/subtree' >&2
	exit 1
fi

for lib; do
	echo "==> Updating $lib"

	if [[ ! -d "external/$lib" ]]; then
		echo "--> External library does not exist (skipping): $lib" >&2
		continue
	fi

	info="$(grep "^$lib " external/subtree.txt)"
	if [[ $info = '' ]]; then
		echo "--> No repository info in subtree.txt (skipping): $lib" >&2
		continue
	fi

	args=( $info );
	repo="${args[1]}"
	branch="${args[2]}"

	git subtree pull --squash --prefix="external/$lib" "$repo" "$branch"
done

