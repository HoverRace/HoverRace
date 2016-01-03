
include(SetWarningLevel)

set(BUILD_SHARED_LIBS TRUE)

add_subdirectory(lua)
set_no_warnings(TARGET liblua)

set(LUA_FOUND TRUE)
include_directories(SYSTEM
	lua/src
	${CMAKE_BINARY_DIR}/external/lua)
set(LUA_LIBRARIES liblua)

set(INSTALL_LUABIND FALSE)
set(BUILD_SHARED_LUABIND TRUE)

add_subdirectory(luabind)
set_no_warnings(TARGET luabind)

add_subdirectory(sqlitecpp)
set_no_warnings(TARGET SQLiteCpp)

set(HR_EXTERNAL_INCLUDE_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/lua/src
	${CMAKE_BINARY_DIR}/external/lua
	${CMAKE_CURRENT_SOURCE_DIR}/luabind
	${CMAKE_CURRENT_SOURCE_DIR}/rapidxml
	${CMAKE_CURRENT_SOURCE_DIR}/sqlitecpp/include
	${CMAKE_CURRENT_SOURCE_DIR}/utfcpp
	PARENT_SCOPE)
