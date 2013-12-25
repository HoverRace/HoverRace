
cmake_minimum_required(VERSION 2.8)

project(hoverrace CXX)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules)

include(CXX11)

check_for_cxx11_compiler(CXX11_COMPILER)
if(CXX11_COMPILER)
	enable_cxx11()
endif()

# Crank up the warning level.
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

# Set up the Boost dependency.
# We only need to list the libraries that have libraries;
# we assume that header-only libraries are available without checking.
include(FindBoost)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.40 REQUIRED COMPONENTS filesystem system thread)
include_directories(${Boost_INCLUDE_DIRS})

if(UNIX)
	include(FindPkgConfig)
	pkg_check_modules(DEPS REQUIRED
		freealut>=1.1
		libcurl>=7.18
		libxdg-basedir>=1.0
		openal>=0.0.8
		sdl2>=2.0
		SDL2_image>=2.0
		SDL2_ttf>=2.0)
	include_directories(${DEPS_INCLUDE_DIRS})
	link_directories(${DEPS_LIBRARY_DIRS})

	#TODO: Bundle Lua and Luabind.
	include(FindLua51)
	if(NOT LUA51_FOUND)
		message(SEND_ERROR "Lua 5.1 is required.")
	endif()
	include_directories(${LUA_INCLUDE_DIR})
	link_directories(${LUA_LIBRARIES})
	set(DEPS_LIBRARIES ${DEPS_LIBRARIES} ${LUA_LIBRARIES})
elseif(WIN32)
	# For Win32 builds, we include the bulk of the dependencies.
	# Boost is the exception since it's simply too big.
	set(DEPS_LIBRARIES
		LiteUnzip
		LiteZip
		optimized SDL2 debug SDL2D
		optimized SDL2main debug SDL2mainD
		optimized SDL2_image debug SDL2_imageD
		alut
		intl
		optimized libcurl debug libcurlD
		optimized lua5.1 debug lua5.1D
		optimized luabind debug luabindD
		optimized yaml debug yamlD)
	include_directories(${CMAKE_SOURCE_DIR}/include)
	link_directories(${CMAKE_SOURCE_DIR}/lib)
	file(GLOB DLLS RELATIVE ${CMAKE_SOURCE_DIR} lib/*.dll)
	file(COPY ${DLLS} DESTINATION ${CMAKE_BINARY_DIR})
endif()

add_subdirectory(engine)
