
function(set_warning_level_direct target gcc_opt msvc_opt)
	if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		set(opt "${gcc_opt}")
	elseif(MSVC)
		set(opt "${msvc_opt}")
	endif()
	set_property(TARGET ${target} APPEND_STRING PROPERTY COMPILE_FLAGS ${opt})
endfunction(set_warning_level_direct)

function(set_full_warnings target)
	set_warning_level_direct(${target} "-Winvalid-pch -Wall" "/W3")
endfunction(set_full_warnings)

function(set_no_warnings target)
	set_warning_level_direct(${target} "-w" "/w")
endfunction(set_no_warnings)

