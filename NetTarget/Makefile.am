
SUBDIRS = Util

BUILT_SOURCES = version.h
CLEANFILES = $(BUILT_SOURCES)

# We extract the version from the resource file for the Windows build.
version.h: Game2/Game2.rc
	$(ECHO) '#pragma once' > $@
	$(GREP) '^ FILEVERSION [0-9,]*' $< | \
		$(SED) -e 's/^ FILEVERSION \([0-9,]*\).*$$/\1/' -e 'y/,/./' | \
		$(AWK) '{ print "#define HR_APP_VERSION \"" $$1 "\"" }' >> \
		$@

