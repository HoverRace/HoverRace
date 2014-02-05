
# Generate version info.
configure_file(Game2/version.in.h ${CONFIG_HEADER_DIR}/hoverrace/hr-version.h
	@ONLY ESCAPE_QUOTES)
if(WIN32)
	set(HR_CLIENT_VERSION_RC ${CONFIG_HEADER_DIR}/hoverrace/Game2.rc)
	configure_file(Game2/Game2.in.rc ${HR_CLIENT_VERSION_RC}
		@ONLY ESCAPE_QUOTES)
endif()

set(GROUPS
	Game2 Game2/HoverScript)

foreach(group ${GROUPS})
	file(GLOB srcs "${group}/*.h" "${group}/*.cpp")
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
set_target_properties(hoverrace PROPERTIES LINKER_LANGUAGE CXX)
include_directories(Game2)
target_link_libraries(hoverrace ${Boost_LIBRARIES} ${DEPS_LIBRARIES}
	hoverrace-engine)
if(WIN32)
	target_link_libraries(hoverrace version comctl32)
endif()

if(NOT WIN32)
	set_property(TARGET hoverrace APPEND PROPERTY COMPILE_DEFINITIONS
		LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")
endif()

# Increase memory for precompiled header.
if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm120")
endif()

# Bump the warning level.
include(SetWarningLevel)
set_full_warnings(TARGET hoverrace)

# Enable precompiled headers.
# Note: Explicitly setting the language to fix misdetection in some cases.
set_target_properties(hoverrace PROPERTIES
	COTIRE_ADD_UNITY_BUILD FALSE)
set_target_properties(hoverrace PROPERTIES
	COTIRE_CXX_PREFIX_HEADER_INIT Game2/StdAfx.h)
cotire(hoverrace LANGUAGES CXX)
