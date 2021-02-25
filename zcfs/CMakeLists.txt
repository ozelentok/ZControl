add_executable(zcfs
	Server.cpp
	main.cpp
)
find_package(FUSE3 REQUIRED)
target_include_directories(zcfs PRIVATE ${FUSE3_INCLUDE_DIR})
target_link_libraries(zcfs PRIVATE commander ${FUSE3_LIBRARIES})
install(TARGETS zcfs DESTINATION bin)
