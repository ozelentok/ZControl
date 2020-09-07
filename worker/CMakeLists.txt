add_executable(worker 
	Worker.cpp
	main.cpp
)
target_link_libraries(worker PRIVATE base common)
