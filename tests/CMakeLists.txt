file(GLOB tests_SRC CONFIGURE_DEPENDS "*.cpp")
add_executable(tests ${tests_SRC})

find_package(Catch2 3 REQUIRED)
target_link_libraries(tests PRIVATE commander worker Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(tests)
enable_testing()
