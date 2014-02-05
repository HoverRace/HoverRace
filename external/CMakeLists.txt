
include(SetWarningLevel)

add_subdirectory(lua)
set_no_warnings(TARGET liblua)
set_no_warnings(TARGET liblua_static)

set(LUA_FOUND TRUE)
include_directories(SYSTEM
	lua/src
	${CMAKE_BINARY_DIR}/external/lua)
set(LUA_LIBRARIES liblua)

add_subdirectory(luabind)
set_no_warnings(TARGET luabind)

set(HR_EXTERNAL_INCLUDE_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/lua/src
	${CMAKE_BINARY_DIR}/external/lua
	${CMAKE_CURRENT_SOURCE_DIR}/luabind
	PARENT_SCOPE)
