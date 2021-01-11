add_library(common STATIC
	BinarySerializer.cpp
	BinaryDeserializer.cpp
	CancellationPipe.cpp
	MessageTransport.cpp
)
target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(common PUBLIC base)
