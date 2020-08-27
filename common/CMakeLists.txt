add_library(common STATIC
	MessageTransport.cpp
)
target_link_libraries(common PRIVATE base)
target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
