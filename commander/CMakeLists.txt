add_library(commander STATIC
	Commander.cpp
	DirEntry.cpp
)
target_include_directories(commander PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(commander PUBLIC common pthread)
if(NOT ANDROID)
	target_link_libraries(commander PUBLIC pthread)
endif()
