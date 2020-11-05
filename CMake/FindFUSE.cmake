find_path(FUSE_INCLUDE_DIR fuse.h)
find_library(FUSE_LIBRARIES	fuse)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("FUSE" DEFAULT_MSG FUSE_INCLUDE_DIR FUSE_LIBRARIES)
mark_as_advanced(FUSE_INCLUDE_DIR FUSE_LIBRARIES)
