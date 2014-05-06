
set(HR_ENGINE_SRCS
	StdAfx.cpp
	StdAfx.h)
source_group("Precompiled Header" FILES ${HR_ENGINE_SRCS})

set(TOPLEVEL_SRCS
	Exception.h
	Vec.h)
source_group("" FILES ${TOPLEVEL_SRCS})
list(APPEND HR_ENGINE_SRCS ${TOPLEVEL_SRCS})

set(GROUPS
	ColorTools
	Control
	Display Display/SDL
	MainCharacter
	MazeCompiler
	Model
	Net
	ObjFac1
	ObjFacTools
	Parcel
	Script Script/Help
	Util Util/yaml
	VideoServices)

foreach(group ${GROUPS})
	file(GLOB srcs "${group}/*.h" "${group}/*.cpp")
	list(APPEND HR_ENGINE_SRCS ${srcs})

	# Arrange the files in Visual Studio by directory.
	if(WIN32)
		string(REGEX REPLACE "/" "\\\\" groupname ${group})
		source_group(${groupname} FILES ${srcs})
	endif()
endforeach()

add_library(hrengine SHARED ${HR_ENGINE_SRCS})
set_target_properties(hrengine PROPERTIES
	LINKER_LANGUAGE CXX
	PROJECT_LABEL "HoverRace Engine")
if(WIN32)
	set_property(TARGET hrengine APPEND PROPERTY COMPILE_DEFINITIONS
		MR_ENGINE  # Enable dllexport.
		YAML_DECLARE_STATIC)  # Use static linking for libYAML.
endif()
include_directories(".")
target_link_libraries(hrengine ${Boost_LIBRARIES} ${DEPS_LIBRARIES}
    libcurl liblua luabind)

# Install prebuilt bundled DLLs into the right locations.
if(WIN32)
	# DLLs that have only release versions.
	set(COMMON_DLLS
		alut intl libfreetype-6 libjpeg-9 libpng16-16 LiteUnzip LiteZip
		OpenAL32 zlib1)
	# DLLs that have both debug and release versions.
	set(CONFIGURED_DLLS
		SDL2 SDL2_image SDL2_ttf)

	foreach(dll ${COMMON_DLLS})
		add_custom_command(TARGET hrengine PRE_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${CMAKE_SOURCE_DIR}/lib/${dll}.dll
			${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/${dll}.dll)
	endforeach()
	foreach(dll ${CONFIGURED_DLLS})
		add_custom_command(TARGET hrengine PRE_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${CMAKE_SOURCE_DIR}/lib/${dll}D.dll
			${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/${dll}D.dll)
		add_custom_command(TARGET hrengine PRE_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${CMAKE_SOURCE_DIR}/lib/${dll}.dll
			${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/${dll}.dll)
	endforeach()
endif(WIN32)

# Bump the warning level.
include(SetWarningLevel)
set_full_warnings(TARGET hrengine)

# Enable precompiled headers.
# Note: Explicitly setting the language to fix misdetection in some cases.
set_target_properties(hrengine PROPERTIES
	COTIRE_ADD_UNITY_BUILD FALSE)
set_target_properties(hrengine PROPERTIES
	COTIRE_CXX_PREFIX_HEADER_INIT StdAfx.h)
cotire(hrengine LANGUAGES CXX)
