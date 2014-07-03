
function(set_warning_level_direct scope gcc_opt msvc_opt)
	if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		set(opt "${gcc_opt}")
	elseif(MSVC)
		set(opt "${msvc_opt}")
	endif()
	set_property(${scope} APPEND_STRING PROPERTY COMPILE_FLAGS " ${opt} ")
endfunction()

function(set_full_warnings)
	set_warning_level_direct("${ARGN}" "-Wall -Winvalid-pch" "/W3")
endfunction()

function(set_no_warnings)
	set_warning_level_direct("${ARGN}" "-w" "/w")
endfunction()

