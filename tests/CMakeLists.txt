include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.8.0
  FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)

file(GLOB tests_SRC CONFIGURE_DEPENDS "*.cpp")
add_executable(tests ${tests_SRC})
target_link_libraries(tests PRIVATE commander worker Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(tests)
enable_testing()
