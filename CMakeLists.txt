cmake_minimum_required(VERSION 3.28.3)
project(http-server VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

### DEPENDENCIES ###

# Google Test Suite
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
)

FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.9.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest googlebenchmark)

include(GoogleTest)

### BUILD PROJECT ####

include_directories(${PROJECT_SOURCE_DIR}/)

set(SOURCES
  
)

set(LIBRARIES
  pthread
)

# Main Build
add_compile_options(-Wall -Werror -Wextra)

add_executable(${PROJECT_NAME} ${SOURCES} Source/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARIES})

add_executable(${PROJECT_NAME}.tsan ${SOURCES} Source/main.cpp)
target_compile_options(${PROJECT_NAME}.tsan PRIVATE -fsanitize=thread) 
target_link_libraries(${PROJECT_NAME}.tsan PRIVATE ${LIBRARIES} tsan) 

### BUILD BENCHMARK ###

set(BENCH_FILES
)

foreach(bench_file ${BENCH_FILES})
  add_executable(${bench_file}_bench ${SOURCES} Benchmarks/${bench_file}_bench.cpp)
  target_link_libraries(${bench_file}_bench PRIVATE benchmark::benchmark ${LIBRARIES})
endforeach()

### BUILD TESTS ### 

enable_testing()

set(TEST_FILES
)

set(GOOGLE_TEST_LIBS 
    GTest::gtest 
    GTest::gtest_main 
    GTest::gmock 
    GTest::gmock_main 
    asan 
    ubsan)

foreach(test_file ${TEST_FILES})
  add_executable(${test_file}_test ${SOURCES} Tests/${test_file}_test.cpp)

  target_compile_options(${test_file}_test PRIVATE -fsanitize=undefined -fsanitize=address)
  target_link_options(${test_file}_test PRIVATE -fsanitize=undefined -fsanitize=address)

  target_link_libraries(${test_file}_test PRIVATE ${GOOGLE_TEST_LIBS} ${LIBRARIES})
  gtest_discover_tests(${test_file}_test) 
endforeach() 
