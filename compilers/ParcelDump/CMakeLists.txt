
set(SRCS
	StdAfx.h
	main.cpp)
source_group(ParcelDump FILES ${SRCS})

add_executable(hoverrace-parceldump ${SRCS})
set_target_properties(hoverrace-parceldump PROPERTIES
	LINKER_LANGUAGE CXX
	PROJECT_LABEL ParcelDump)
target_link_libraries(hoverrace-parceldump ${Boost_LIBRARIES} ${DEPS_LIBRARIES}
	hrengine)

if(NOT WIN32)
	set_property(TARGET hoverrace-parceldump
		APPEND PROPERTY COMPILE_DEFINITIONS
		LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")
endif()

# Bump the warning level.
include(SetWarningLevel)
set_full_warnings(TARGET hoverrace-parceldump)

# Note: Even though we have a standard StdAfx.h, we don't use bother with
#       precompiled headers since there's only a single source file.
