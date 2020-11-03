add_library(base STATIC
	AddressInfo.cpp 
	AddressInfoErrorCategory.cpp 
	TcpSocket.cpp
	ThreadPool.cpp
)
target_include_directories(base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
