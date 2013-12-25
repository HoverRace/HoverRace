
file(GLOB_RECURSE UTIL_SRCS Util/*.h Util/*.cpp)

set(HR_ENGINE_SRCS
	StdAfx.h
	${UTIL_SRCS})

add_library(hoverrace-engine SHARED ${HR_ENGINE_SRCS})
set_target_properties(hoverrace-engine PROPERTIES LINKER_LANGUAGE CXX)
if(WIN32)
	set_property(TARGET hoverrace-engine APPEND PROPERTY COMPILE_DEFINITIONS "MR_ENGINE")
endif()
include_directories(".")
target_link_libraries(hoverrace-engine ${Boost_LIBRARIES} ${DEPS_LIBRARIES})
