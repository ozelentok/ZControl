add_executable(tests test.cpp)

find_package(doctest REQUIRED)
target_link_libraries(tests PRIVATE worker server doctest::doctest)
