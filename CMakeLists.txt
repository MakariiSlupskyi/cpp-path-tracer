cmake_minimum_required(VERSION 3.20)

project(PathTracer)

set(CMAKE_CXX_STANDARD 20)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
  glfw
  glad::glad
)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/src/shader/shader.frag"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/shader")