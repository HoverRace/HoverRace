# vim:ts=3:sw=3:expandtab

# This is a customized version of GETTEXT_CREATE_TRANSLATIONS from
# FindGettext.cmake (bundled with CMake).
#
# We insert a cleanup step between the msgmerge and msgfmt steps:
# Post-process the .po file to reduce the number of changes.
#
# Before, this step was appended to the list of commands, which had the
# side-effect of modifying the .po file *after* msgfmt had already run,
# so CMake would *always* rebuild the file every time since the .gmo file
# would be older than the .po file.

macro(HR_GETTEXT_CREATE_TRANSLATIONS _potFile _firstPoFileArg)
   # make it a real variable, so we can modify it here
   set(_firstPoFile "${_firstPoFileArg}")

   set(_gmoFiles)
   get_filename_component(_potName ${_potFile} NAME)
   string(REGEX REPLACE "^(.+)(\\.[^.]+)$" "\\1" _potBasename ${_potName})
   get_filename_component(_absPotFile ${_potFile} ABSOLUTE)

   set(_addToAll)
   if(${_firstPoFile} STREQUAL "ALL")
      set(_addToAll "ALL")
      set(_firstPoFile)
   endif()

   foreach (_currentPoFile ${_firstPoFile} ${ARGN})
      get_filename_component(_absFile ${_currentPoFile} ABSOLUTE)
      get_filename_component(_abs_PATH ${_absFile} PATH)
      get_filename_component(_lang ${_absFile} NAME_WE)
      set(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo)

      add_custom_command(
         OUTPUT ${_gmoFile}
         COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --quiet --update --backup=none -s ${_absFile} ${_absPotFile}
         COMMAND ${CMAKE_COMMAND} "-DPO_FILE=${_absFile}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/CleanPo.cmake"
         COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_gmoFile} ${_absFile}
         DEPENDS ${_absPotFile} ${_absFile}
      )

      install(FILES ${_gmoFile} DESTINATION share/locale/${_lang}/LC_MESSAGES RENAME ${_potBasename}.mo)
      set(_gmoFiles ${_gmoFiles} ${_gmoFile})

   endforeach ()

   if(NOT TARGET translations)
      add_custom_target(translations)
   endif()

   _GETTEXT_GET_UNIQUE_TARGET_NAME(translations uniqueTargetName)

   add_custom_target(${uniqueTargetName} ${_addToAll} DEPENDS ${_gmoFiles})

   add_dependencies(translations ${uniqueTargetName})

endmacro()

