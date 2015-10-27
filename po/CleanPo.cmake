
# PO_FILE:string is passed-in from CMakeLists.txt.

message(STATUS "Cleaning up ${PO_FILE}")

file(READ "${PO_FILE}" po_in)

# If a comment has more than one filename, split into multiple lines.
# We assume that there are at most two filenames on a single line.
string(REGEX REPLACE
	"(#: [^ \r\n]+) ([^\r\n]+)(\r?\n)" "\\1\\3#: \\2\\3"
	po_split "${po_in}")

# Remove the line number from the comment, leaving only the filename.
# The line numbers change often so they clutter the history.
# GNU gettext 0.19 has an --add-location=file option, but we're still on
# 0.18 compatibility, so we'll do it ourselves.
string(REGEX REPLACE
	"(#: [^:]+):[0-9]+([^\r\n]*\r?\n)" "\\1\\2"
	po_out "${po_split}")

file(WRITE "${PO_FILE}" "${po_out}")

