
find_path(YAML_INCLUDE_DIR NAMES yaml.h)
find_library(YAML_LIBRARIES NAMES yaml libyaml)
find_package_handle_standard_args(YAML DEFAULT_MSG
	YAML_LIBRARIES YAML_INCLUDE_DIR)
mark_as_advanced(YAML_INCLUDE_DIR YAML_LIBRARIES)
