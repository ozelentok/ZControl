add_executable(server
	DirEntry.cpp
	Commander.cpp
	main.cpp
)
target_link_libraries(server PRIVATE base common pthread)
