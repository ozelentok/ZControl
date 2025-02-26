add_library(worker STATIC
  FileCommandsHandler.cpp
  DirCommandsHandler.cpp
  Worker.cpp
)
target_include_directories(worker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(worker PUBLIC base common)
