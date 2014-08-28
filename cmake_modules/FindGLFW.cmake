# Try to find GLFW. Once done, this will define:
#
#   GLFW_FOUND - variable which returns the result of the search
#   GLFW_INCLUDE_DIRS - include directory
#   GLFW_LIBRARIES - options for the linker
#   currently uses the dynamic libraries!! could be changed!!
#   Also should look for the approprate 64bit or 32bit libs

if(WIN32)
  if(MSVC)
    #set(GLFW_INCLUDE_DIR C:/)
    #set(GLFW_LIBRARY glfw3.lib)
    find_path(GLFW_INCLUDE_DIRS
      NAMES GLFW/glfw3.h
      PATHS
      "${GLFW_LOCATION}/include"
      "$ENV{GLFW_LOCATION}/include"
      "${PROJECT_SOURCE_DIR}/libraries/glfw/include")
    find_library(GLFW_LIBRARIES NAMES glfw3dll.lib
      PATHS
      "${GLFW_LOCATION}/lib-msvc120"
      "$ENV{GLFW_LOCATION}/lib-msvc120"
      "${PROJECT_SOURCE_DIR}/libraries/glfw/lib-msvc120") #no option for static vs dynamic link
  elseif(CYGWIN)
    message(FATAL_ERROR "Cygwin not yet supported!")
  endif()
elseif(UNIX)
  find_package(PkgConfig)
  pkg_search_module(GLFW REQUIRED glfw3)
elseif(APPLE)
  find_package(PkgConfig)
  if(PKG_CONFIG_FOUND)
    pkg_search_module(GLFW REQUIRED glfw3)
  else()
    message(FATAL_ERROR "Mac OS X builds without pkg-config not yet supported")
  endif()
endif()

message("Path found for GLFW include dir: " ${GLFW_INCLUDE_DIRS})
message("Path found for GLFW library: " ${GLFW_LIBRARIES})
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLFW DEFAULT_MSG
  GLFW_INCLUDE_DIRS
  GLFW_LIBRARIES)
