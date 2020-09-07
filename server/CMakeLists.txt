add_executable(server
	Commander.cpp
	main.cpp
)
target_link_libraries(server PRIVATE base common)
