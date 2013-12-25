
set(HR_ENGINE_SRCS
	StdAfx.cpp
	StdAfx.h)

set(GROUPS
	Util Util/yaml)

foreach(group ${GROUPS})
	file(GLOB srcs "${group}/*.h" "${group}/*.cpp")
	set(HR_ENGINE_SRCS ${HR_ENGINE_SRCS} ${srcs})

	# Arrange the files in Visual Studio by directory.
	if(WIN32)
		string(REGEX REPLACE "/" "\\\\" groupname ${group})
		source_group(${groupname} FILES ${srcs})
	endif()
endforeach()

add_library(hoverrace-engine SHARED ${HR_ENGINE_SRCS})
set_target_properties(hoverrace-engine PROPERTIES LINKER_LANGUAGE CXX)
if(WIN32)
	set_property(TARGET hoverrace-engine APPEND PROPERTY COMPILE_DEFINITIONS
		MR_ENGINE  # Enable dllexport.
		YAML_DECLARE_STATIC)  # Use static linking for libYAML.
endif()
include_directories(".")
target_link_libraries(hoverrace-engine ${Boost_LIBRARIES} ${DEPS_LIBRARIES})

set_target_properties(hoverrace-engine PROPERTIES
	COTIRE_ADD_UNITY_BUILD FALSE)
set_target_properties(hoverrace-engine PROPERTIES
	COTIRE_CXX_PREFIX_HEADER_INIT StdAfx.h)
cotire(hoverrace-engine)
