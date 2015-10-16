
set(PO_SRCS
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
source_group(Translations FILES ${PO_SRCS})
set(POT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/hoverrace.pot)
file(STRINGS potfiles.txt SRCS NO_HEX_CONVERSION)
file(STRINGS linguas.txt LINGUAS NO_HEX_CONVERSION)

include(FindXGettext)

xgettext_create_pot_file(${POT_FILE} CPP
	INPUT ${SRCS} "--boost" "--sort-by-file"
		"--msgid-bugs-address" "https://github.com/HoverRace/HoverRace"
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	KEYWORDS "_:1,1t" "_:1c,2,2t" "_:1,2,3t" "_:1c,2,3,4t"
	PACKAGE_NAME hoverrace
	COPYRIGHT_HOLDER "HoverRace contributors")

gettext_create_translations(${POT_FILE} ${PO_SRCS})

# Install the compiled locales into the resource directory so that they're
# available during development.
foreach(lang ${LINGUAS})
	set(gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${lang}.gmo)
	set(moFile
		${CMAKE_SOURCE_DIR}/share/locale/${lang}/LC_MESSAGES/hoverrace.mo)

	add_custom_command(TARGET translations_1 POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${gmoFile} ${moFile})
endforeach()
