
message(STATUS "Cleaning up ${POT_FILE}")

file(READ "${POT_FILE}" pot_in)

# Remove the creation date header since the .pot file is regenerated during
# the build, and the updated header propagates to the .po files.
string(REGEX REPLACE "\"POT-Creation-Date:[^\r\n]*\r?\n" "" pot_in "${pot_in}")

file(WRITE "${POT_FILE}" "${pot_in}")
