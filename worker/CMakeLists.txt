add_executable(worker 
	FileCommandsHandler.cpp
	DirCommandsHandler.cpp
	Worker.cpp
	main.cpp
)
target_link_libraries(worker PRIVATE base common pthread)
