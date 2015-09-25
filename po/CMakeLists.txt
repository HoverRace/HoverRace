
set(SRCS
	de.po
	en_AU.po
	en_GB.po
	en_NZ.po
	en_US@piglatin.po
	es.po
	et.po
	fi.po
	fr.po
	ja.po
	ko.po
	nb.po
	nl.po
	pt.po
	pt_BR.po
	ro.po
	sr.po
	tlh.po)
source_group(Translations FILES ${SRCS})

GETTEXT_CREATE_TRANSLATIONS(hoverrace.pot ALL ${SRCS})
