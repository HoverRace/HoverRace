
set(POT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/po/hoverrace.pot)

message(STATUS "Cleaning up ${POT_FILE}")

file(READ "${POT_FILE}" pot_in)
string(REGEX REPLACE "\"POT-Creation-Date:[^\r\n]*\r?\n" "" pot_out "${pot_in}")
file(WRITE "${POT_FILE}" "${pot_out}")
