cmake_minimum_required(VERSION 3.31)
project(Benderer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra)
    add_compile_options($<$<CONFIG:Release>:-O3>)
    add_compile_options($<$<CONFIG:Release>:-march=native>)
endif()

file(GLOB_RECURSE BENDERER_SOURCES CONFIGURE_DEPENDS
    "source/*.h"
    "source/*.cpp"
)

add_executable(Benderer ${BENDERER_SOURCES})