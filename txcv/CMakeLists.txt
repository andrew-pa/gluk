cmake_minimum_required(VERSION 3.0)
project(txcv)

file(GLOB SOURCES src/*.cpp)
include_directories("../inc/" "${CMAKE_CURRENT_SOURCE_DIR}/inc")
add_definitions(${GLOBAL_DEFINES})
add_executable(txcv ${SOURCES})
target_link_libraries(txcv gluk)
