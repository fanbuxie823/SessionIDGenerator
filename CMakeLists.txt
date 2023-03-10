cmake_minimum_required(VERSION 3.20)
project(cppTest)

set(CMAKE_CXX_STANDARD 20)

add_executable(cppTest main.cpp)
add_compile_definitions(cppTest PUBLIC
  $<$<CONFIG:Debug>:DEBUG>
  )