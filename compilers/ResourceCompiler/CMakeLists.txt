
set(SRCS
	StdAfx.cpp
	StdAfx.h)
source_group("Precompiled Header" FILES ${SRCS})

set(srcs
	BitmapHelper.cpp
	BitmapHelper.h
	ResActorBuilder.cpp
	ResActorBuilder.h
	ResBitmapBuilder.cpp
	ResBitmapBuilder.h
	ResourceCompiler.cpp
	ResourceLibBuilder.cpp
	ResourceLibBuilder.h
	ResSoundBuilder.cpp
	ResSoundBuilder.h
	ResSpriteBuilder.cpp
	ResSpriteBuilder.h
	TextParsing.cpp
	TextParsing.h)
source_group("Resource Compiler" FILES ${srcs})
list(APPEND SRCS ${srcs})

add_executable(hoverrace-resource-compiler ${SRCS})
set_target_properties(hoverrace-resource-compiler PROPERTIES
	LINKER_LANGUAGE CXX
	PROJECT_LABEL ResourceCompiler)
target_link_libraries(hoverrace-resource-compiler ${Boost_LIBRARIES}
	${DEPS_LIBRARIES} hrengine)

if(NOT WIN32)
	set_property(TARGET hoverrace-resource-compiler
		APPEND PROPERTY COMPILE_DEFINITIONS
		LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")
endif()

# Bump the warning level.
include(SetWarningLevel)
set_full_warnings(TARGET hoverrace-resource-compiler)
