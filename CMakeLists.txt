cmake_minimum_required (VERSION 2.6)
project (lao)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(Lua51 REQUIRED)
find_package(LibAo REQUIRED)
include_directories(${LUA_INCLUDE_DIR} ${LIBAO_INCLUDE_DIRS})

add_library(ao SHARED src/lao.c)
target_link_libraries(ao ${LIBAO_LIBRARIES})