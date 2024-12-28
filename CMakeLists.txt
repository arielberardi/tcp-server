cmake_minimum_required(VERSION 3.28.3)
project(tcp-server VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

### BUILD PROJECT ####
include_directories(${PROJECT_SOURCE_DIR}/)

set(SOURCES
  
)

set(LIBRARIES
  pthread
)

add_compile_options(-Wall -Werror -Wextra)
add_executable(${PROJECT_NAME} ${SOURCES} Source/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARIES})
