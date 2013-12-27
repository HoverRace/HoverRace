
set(GROUPS
	Game2 Game2/HoverScript)

foreach(group ${GROUPS})
	file(GLOB srcs "${group}/*.h" "${group}/*.cpp")
	set(HR_CLIENT_SRCS ${HR_CLIENT_SRCS} ${srcs})

	# Arrange the files in Visual Studio by directory.
	if(WIN32)
		string(REGEX REPLACE "/" "\\\\" groupname ${group})
		source_group(${groupname} FILES ${srcs})
	endif()
endforeach()

add_executable(hoverrace WIN32 ${HR_CLIENT_SRCS})
set_target_properties(hoverrace PROPERTIES LINKER_LANGUAGE CXX)
include_directories(Game2)
target_link_libraries(hoverrace ${Boost_LIBRARIES} ${DEPS_LIBRARIES}
	hoverrace-engine)
if(WIN32)
	target_link_libraries(hoverrace version comctl32)
endif()

# Increase memory for precompiled header.
if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm120")
endif()

set_target_properties(hoverrace PROPERTIES
	COTIRE_ADD_UNITY_BUILD FALSE)
set_target_properties(hoverrace PROPERTIES
	COTIRE_CXX_PREFIX_HEADER_INIT Game2/StdAfx.h)
cotire(hoverrace)
