
set(HR_BUILD_UTILS TRUE CACHE BOOL "Build extra command-line utilities")

if(HR_BUILD_UTILS)
	add_subdirectory(ParcelDump)
endif()

