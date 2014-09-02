
# Generate version info.
configure_file(Game2/version.in.h ${CONFIG_HEADER_DIR}/hoverrace/hr-version.h
	@ONLY ESCAPE_QUOTES)
if(WIN32)
	set(HR_CLIENT_VERSION_RC ${CONFIG_HEADER_DIR}/hoverrace/Game2.rc)
	configure_file(Game2/Game2.in.rc ${HR_CLIENT_VERSION_RC}
		@ONLY ESCAPE_QUOTES)
endif()

set(HR_CLIENT_SRCS
	StdAfx.cpp
	StdAfx.h)
source_group("Precompiled Header" FILES ${HR_CLIENT_SRCS})

set(GROUPS
	Game2 Game2/HoverScript)

foreach(group ${GROUPS})
	file(GLOB srcs "${group}/*.h" "${group}/*.cpp")
	file(GLOB excludes "${group}/.*.h" "${group}/.*.cpp")
	if(excludes)
		list(REMOVE_ITEM srcs ${excludes})
	endif()
	list(APPEND HR_CLIENT_SRCS ${srcs})

	# Arrange the files in Visual Studio by directory.
	if(WIN32)
		string(REGEX REPLACE "/" "\\\\" groupname ${group})
		source_group(${groupname} FILES ${srcs})
	endif()
endforeach()

if(WIN32)
	file(GLOB srcs Game2/*.ico Game2/*.bmp)
	list(APPEND srcs ${HR_CLIENT_VERSION_RC})
	list(APPEND HR_CLIENT_SRCS ${srcs})
	source_group(Resources FILES ${srcs})
endif()

add_executable(hoverrace WIN32 ${HR_CLIENT_SRCS})
set_target_properties(hoverrace PROPERTIES
	LINKER_LANGUAGE CXX
	PROJECT_LABEL HoverRace)
target_link_libraries(hoverrace ${Boost_LIBRARIES} ${DEPS_LIBRARIES}
	hrengine)
if(WIN32)
	target_link_libraries(hoverrace version comctl32)
endif()

if(NOT WIN32)
	set_property(TARGET hoverrace APPEND PROPERTY COMPILE_DEFINITIONS
		LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")
endif()

# Increase memory for precompiled header.
if(MSVC)
	# Using /Xm option instead of /Zm for the Nov 2013 CTP compiler.
	# https://connect.microsoft.com/VisualStudio/feedback/details/808945/torino-error-c3859-c1076
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Xm300")
endif()

# Bump the warning level.
include(SetWarningLevel)
set_full_warnings(TARGET hoverrace)
# Temporarily disable deprecation warnings on Clang; the warnings are coming
# from the Luabind headers, which we're not touching for now.
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set_property(TARGET hoverrace APPEND_STRING PROPERTY COMPILE_FLAGS
		" -Wno-deprecated-declarations ")
endif()

# Enable precompiled headers.
# Note: Explicitly setting the language to fix misdetection in some cases.
set_target_properties(hoverrace PROPERTIES
	COTIRE_ADD_UNITY_BUILD FALSE)
set_target_properties(hoverrace PROPERTIES
	COTIRE_CXX_PREFIX_HEADER_INIT StdAfx.h)
cotire(hoverrace LANGUAGES CXX)

# Install convenience wrapper scripts.
if(UNIX)
	include(CopyWrapperScript)
	copy_wrapper_script(hr.in hr)
	copy_wrapper_script(hr-gdb.in hr-gdb)
	copy_wrapper_script(hr-valgrind.in hr-valgrind)
endif()
