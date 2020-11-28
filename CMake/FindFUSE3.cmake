find_path(FUSE3_INCLUDE_DIR fuse3/fuse_lowlevel.h)
set(FUSE3_INCLUDE_DIR "${FUSE3_INCLUDE_DIR}/fuse3")
find_library(FUSE3_LIBRARIES fuse3)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("FUSE3" DEFAULT_MSG FUSE3_INCLUDE_DIR FUSE3_LIBRARIES)
mark_as_advanced(FUSE3_INCLUDE_DIR FUSE3_LIBRARIES)
