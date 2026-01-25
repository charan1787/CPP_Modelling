cmake_minimum_required(VERSION 3.16)
project(gpu_model_simple_active CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

add_library(gpu_model
  src/model.cpp
)
target_include_directories(gpu_model PUBLIC src)

add_executable(demo
  app/main.cpp
)
target_link_libraries(demo PRIVATE gpu_model)

enable_testing()
add_executable(tests
  tests/test_conformance.cpp
  tests/test_random.cpp
)
target_link_libraries(tests PRIVATE gpu_model gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)
