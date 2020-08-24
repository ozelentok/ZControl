add_library(base STATIC
	AddressInfo.cpp 
	AddressInfoErrorCategory.cpp 
	TcpSocket.cpp
)
target_include_directories(base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
