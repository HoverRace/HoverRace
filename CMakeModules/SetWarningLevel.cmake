
macro(set_warning_level_direct target val)
	set_property(TARGET ${target} APPEND_STRING PROPERTY COMPILE_FLAGS ${val})
endmacro(set_warning_level_direct)

macro(set_full_warnings target)
	if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		set_warning_level_direct(${target} "-Winvalid-pch -Wall")
	elseif(MSVC)
		set_warning_level_direct(${target} "/W3")
	endif()
endmacro(set_full_warnings)

macro(set_no_warnings target)
	if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		set_warning_level_direct(${target} "-w")
	elseif(MSVC)
		set_warning_level_direct(${target} "/W0")
	endif()
endmacro(set_no_warnings)

