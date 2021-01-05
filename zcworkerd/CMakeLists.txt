add_executable(zcworkerd
	main.cpp
)
target_link_libraries(zcworkerd PRIVATE base common worker)
