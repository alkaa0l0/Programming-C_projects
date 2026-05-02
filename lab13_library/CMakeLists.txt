cmake_minimum_required(VERSION 3.10)
project(crypto_project C)


set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)


set(LIB_SOURCES ${SRC_DIR}/crypto.c)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


add_library(crypto_lib ${TYPE} ${LIB_SOURCES})


add_executable(crypto_program ${SRC_DIR}/main.c)
target_link_libraries(crypto_program crypto_lib)
