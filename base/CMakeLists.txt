add_library(base STATIC
  AddressInfo.cpp
  AddressInfoErrorCategory.cpp
  Pipe.cpp
  TcpSocket.cpp
  ThreadPool.cpp
  Logger.cpp
)
target_include_directories(base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
