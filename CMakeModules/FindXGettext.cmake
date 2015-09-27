# Copyright (C) 2013 Canonical Ltd
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# This package provides macros that wrap the xgettext program.
# 
# An example of common usage is:
#
#    set(
#        POT_FILE
#        "${CMAKE_CURRENT_SOURCE_DIR}/${GETTEXT_PACKAGE}.pot"
#    )
#
#    file(
#        GLOB_RECURSE SRC_FILES
#        RELATIVE ${CMAKE_SOURCE_DIR}
#        ${SOURCE_DIR}/*.cpp
#        ${SOURCE_DIR}/*.c
#        ${SOURCE_DIR}/*.h
#    )
#
#    xgettext_create_pot_file(
#        ${POT_FILE}
#        CPP
#        QT
#        INPUT ${SOURCES}
#        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#        ADD_COMMENTS "TRANSLATORS"
#        KEYWORDS "_" "N_"
#        PACKAGE_NAME ${GETTEXT_PACKAGE}
#        COPYRIGHT_HOLDER "Canonical Ltd."
#    )

find_package(Gettext REQUIRED)

find_program(XGETTEXT_EXECUTABLE xgettext)

if(XGETTEXT_EXECUTABLE)
    execute_process(
        COMMAND ${XGETTEXT_EXECUTABLE} --version
        OUTPUT_VARIABLE xgettext_version
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (xgettext_version MATCHES "^xgettext \\(.*\\) [0-9]")
        string(
            REGEX REPLACE "^xgettext \\([^\\)]*\\) ([0-9\\.]+[^ \n]*).*" "\\1"
            XGETTEXT_VERSION_STRING "${xgettext_version}"
        )
    endif()
    unset(xgettext_version)
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    XGettext
    REQUIRED_VARS XGETTEXT_EXECUTABLE
    VERSION_VAR XGETTEXT_VERSION_STRING
)

function(APPEND_EACH LISTNAME GLUE OUTPUT)
    set(_tmp_list "")
    foreach(VAL ${${LISTNAME}})
        list(APPEND _tmp_list "${GLUE}${VAL}")
    endforeach(VAL ${${LISTNAME}})
    set(${OUTPUT} "${_tmp_list}" PARENT_SCOPE)
endfunction()

function(XGETTEXT_CREATE_POT_FILE _potFile)
    set(_options ALL QT CPP)
    set(_oneValueArgs ADD_COMMENTS PACKAGE_NAME COPYRIGHT_HOLDER WORKING_DIRECTORY)
    set(_multiValueArgs INPUT KEYWORDS)

    cmake_parse_arguments(_ARG "${_options}" "${_oneValueArgs}" "${_multiValueArgs}" ${ARGN})
    
    set(_QT "")
    if(_ARG_QT)
        set(_QT "--qt")
    endif()
    
    set(_CPP "")
    if(_ARG_CPP)
        set(_CPP "--c++")
    endif()
    
    set(_KEYWORD "")
    if(_ARG_KEYWORDS)
        append_each(_ARG_KEYWORDS "--keyword=" _KEYWORD)
    endif()
    
    set(_ADD_COMMENTS "")
    if(_ARG_ADD_COMMENTS)
        set(_ADD_COMMENTS --add-comments="${_ARG_ADD_COMMENTS}")
    endif()
    
    set(_PACKAGE_NAME "")
    if(_ARG_PACKAGE_NAME)
        set(_PACKAGE_NAME --package-name="${_ARG_PACKAGE_NAME}")
    endif()
    
    set(_COPYRIGHT_HOLDER "")
    if(_ARG_COPYRIGHT_HOLDER)
        set(_COPYRIGHT_HOLDER --copyright-holder="${_ARG_COPYRIGHT_HOLDER}")
    endif()

    add_custom_command(
        OUTPUT "${_potFile}"
        COMMAND ${XGETTEXT_EXECUTABLE} --output="${_potFile}" ${_KEYWORD} ${_PACKAGE_NAME} ${_COPYRIGHT_HOLDER} ${_QT} ${_CPP} ${_ADD_COMMENTS} ${_ARG_INPUT}
        WORKING_DIRECTORY ${_ARG_WORKING_DIRECTORY}
    )
    
    _GETTEXT_GET_UNIQUE_TARGET_NAME(_potFile _uniqueTargetName)
    
    if(_ARG_ALL)
        add_custom_target(${_uniqueTargetName} ALL DEPENDS ${_potFile})
    else()
        add_custom_target(${_uniqueTargetName} DEPENDS ${_potFile})
    endif()

endfunction()
