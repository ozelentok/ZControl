add_executable(zcfs
	Server.cpp
	main.cpp
)
find_package(FUSE REQUIRED)
target_link_libraries(zcfs PRIVATE commander fuse)
