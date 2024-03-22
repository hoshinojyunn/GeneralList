cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
project(Py-Liked-List)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_CXX_COMPILER /usr/bin/clang++)
# include your headers
include_directories(${CMAKE_SOURCE_DIR}/includes)
add_subdirectory(src)
