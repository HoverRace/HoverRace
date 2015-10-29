
set(POT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/hoverrace.pot)
file(STRINGS potfiles.txt SRCS NO_HEX_CONVERSION)
file(STRINGS linguas.txt LINGUAS NO_HEX_CONVERSION)

foreach(lang ${LINGUAS})
	list(APPEND PO_SRCS "${lang}.po")
endforeach()
source_group(Translations FILES ${PO_SRCS})

include(FindXGettext)

xgettext_create_pot_file(${POT_FILE} CPP
	INPUT ${SRCS} "--boost" "--sort-by-file"
		"--msgid-bugs-address" "https://github.com/HoverRace/HoverRace"
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	KEYWORDS "_:1,1t" "_:1c,2,2t" "_:1,2,3t" "_:1c,2,3,4t"
	PACKAGE_NAME hoverrace
	COPYRIGHT_HOLDER "HoverRace contributors")

gettext_create_translations(${POT_FILE} ALL ${PO_SRCS})

# Post-process the .pot file to reduce the number of changes.
add_custom_command(OUTPUT ${POT_FILE}
	COMMAND ${CMAKE_COMMAND} "-DPOT_FILE=${POT_FILE}" -P po/CleanPot.cmake
	APPEND)

# Install the compiled locales into the resource directory so that they're
# available during development.
foreach(lang ${LINGUAS})
	set(poFile ${CMAKE_CURRENT_SOURCE_DIR}/${lang}.po)
	set(gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${lang}.gmo)
	set(moFile
		${CMAKE_SOURCE_DIR}/share/locale/${lang}/LC_MESSAGES/hoverrace.mo)

	# Post-process the .po file to reduce the number of changes.
	# Note that we hang this step on the .gmo file output, since that's when
	# the .po file is is updated as a side-effect.
	add_custom_command(OUTPUT ${gmoFile}
		COMMAND ${CMAKE_COMMAND} "-DPO_FILE=${poFile}"
			-P "${CMAKE_CURRENT_SOURCE_DIR}/CleanPo.cmake"
		APPEND)

	# Copy the compiled .mo file to where the client will find it when
	# debugging.
	add_custom_command(TARGET translations_1 POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${gmoFile} ${moFile})
endforeach()

