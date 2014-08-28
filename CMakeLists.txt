cmake_minimum_required(VERSION 3.0)
project(gluk)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
  message(FATAL_ERROR "Yo! Out of Source builds only!")
endif()

set(GLOBAL_DEFINES "-DUNICODE -D_UNICODE") #make sure we use unicode

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
add_definitions(${GLOBAL_DEFINES})

file(GLOB SOURCES src/*.cpp inc/*.h)

add_subdirectory(txcv)
add_library(gluk ${SOURCES})
add_executable(test_gluk test_gluk.cpp)
target_link_libraries(test_gluk gluk ${OPENGL_LIBRARIES} ${GLFW_LIBRARIES} ${GLEW_LIBRARIES})
