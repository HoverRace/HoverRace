
set(SRCS
	StdAfx.h
	main.cpp)
source_group(MazeCompiler FILES ${SRCS})

add_executable(hoverrace-maze-compiler ${SRCS})
set_target_properties(hoverrace-maze-compiler PROPERTIES
	LINKER_LANGUAGE CXX
	PROJECT_LABEL MazeCompiler)
target_link_libraries(hoverrace-maze-compiler ${Boost_LIBRARIES}
	${DEPS_LIBRARIES} hoverrace-engine)

if(NOT WIN32)
	set_property(TARGET hoverrace-maze-compiler
		APPEND PROPERTY COMPILE_DEFINITIONS
		LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")
endif()

# Bump the warning level.
include(SetWarningLevel)
set_full_warnings(TARGET hoverrace-maze-compiler)

# Note: Even though we have a standard StdAfx.h, we don't use bother with
#       precompiled headers since there's only a single source file.
