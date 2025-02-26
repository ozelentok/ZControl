file(GLOB tests_SRC CONFIGURE_DEPENDS "*.cpp")
add_executable(tests ${tests_SRC})

find_package(doctest REQUIRED)
target_link_libraries(tests PRIVATE commander worker doctest::doctest)

include(CTest)
include(doctest)
doctest_discover_tests(tests)
enable_testing()
