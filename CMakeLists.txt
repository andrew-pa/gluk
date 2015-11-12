cmake_minimum_required(VERSION 3.0)
project(gluk)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
  message(FATAL_ERROR "Yo! Out of Source builds only!")
endif()

if(MINGW)
	set(GLOBAL_DEFINES "-DMINGW")
endif()

set(GLOBAL_DEFINES "${GLOBAL_DEFINES} -DUNICODE -D_UNICODE") #make sure we use unicode

find_package(OpenGL REQUIRED)

set(SHARE_DIR Source/share)
include("$ENV{HOMEDRIVE}$ENV{HOMEPATH}/${SHARE_DIR}/CMakeSetup.txt" OPTIONAL RESULT_VARIABLE SHR_RES)
if(SHR_RES STREQUAL NOTFOUND)
include($ENV{SHARE_DIR}/CMakeSetup.txt OPTIONAL RESULT_VARIABLE SHR_RES)
endif()

if(SHR_RES STREQUAL NOTFOUND)
  message("No share, stuff installed to system?")
  #old, questionable way of doing things
  #find_package(GLEW REQUIRED)
  #find_package(GLFW REQUIRED)
  #set(GLI_PACKAGE_DIR "C:/Users/andre_000/Source/Libraries/gli-0.5.1.0")
  #set(GLM_INCLUDE_DIR "C:/Users/andre_000/Source/Libraries/glm")
  #find_package(GLI REQUIRED)
  #include_directories(${GLFW_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${GLI_INCLUDE_DIRS} ${GLM_INCLUDE_DIR})
else()
  include_directories(${SHARE_INCLUDE})
  message("Using share (INC: ${SHARE_INCLUDE} LIB: ${SHARE_LIBDIR})")
endif()


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc/gluk ${CMAKE_CURRENT_SOURCE_DIR}/inc/soil)
add_definitions(${GLOBAL_DEFINES})

file(GLOB SOIL_SRC src/soil/* inc/soil/*)
add_library(SOIL ${SOIL_SRC})

file(GLOB SOURCES src/*.cpp inc/*/*.h)

add_library(gluk ${SOURCES})
add_executable(test_gluk test_gluk.cpp)
if(SHR_RES)
	target_link_libraries(test_gluk gluk ${OPENGL_LIBRARIES} ${SHARE_LIBDIR}/glfw3dll.lib ${SHARE_LIBDIR}/glew32.lib ${SHARE_LIBDIR}/freetype.lib SOIL)
else()
	target_link_libraries(test_gluk gluk ${OPENGL_LIBRARIES} glfw3 glew32 freetype SOIL)
endif()
set_property(TARGET gluk PROPERTY CXX_STANDARD 14)
set_property(TARGET test_gluk PROPERTY CXX_STANDARD 14)
file(GLOB SHADERS src/*.glsl)
message("S" ${SHADERS})
if(NOT MINGW)
    file(TO_NATIVE_PATH ${PROJECT_BINARY_DIR}/$<CONFIG> NATIVE_SHADER_OUT_DIR)
    foreach(SHADER ${SHADERS})
      message("SS" ${SHADER})
      file(TO_NATIVE_PATH ${SHADER} NATIVEP_SHADER)
      add_custom_command(TARGET test_gluk POST_BUILD
	  COMMAND copy ARGS /Y ${NATIVEP_SHADER} ${NATIVE_SHADER_OUT_DIR})
	endforeach(SHADER)
endif()
