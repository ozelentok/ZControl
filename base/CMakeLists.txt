add_library(zcontrol-base STATIC
	AddressInfo.cpp 
	GAIErrorCategory.cpp 
	TcpSocket.cpp
)
target_include_directories(zcontrol-base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
