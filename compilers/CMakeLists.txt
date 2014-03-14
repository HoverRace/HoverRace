
set(HR_BUILD_UTILS FALSE CACHE BOOL "Build extra command-line utilities")

if(HR_BUILD_UTILS)
	add_subdirectory(MazeCompiler)
	add_subdirectory(ParcelDump)
	add_subdirectory(ResourceCompiler)
endif()

