# Try to find GLEW. Once done, this will define:
#
#   GLEW_FOUND - variable which returns the result of the search
#   GLEW_INCLUDE_DIRS - include directory
#   GLEW_LIBRARIES - options for the linker
#   GLEW_DEBUG_LIBRARIES - options for the linker in debug mode
#   currently uses the dynamic libraries!! could be changed!!
#   Also should look for the approprate 64bit or 32bit libs
#   Also should be able to generate different lib paths for Debug or Release
#     as is provided by GLEW when built
if(WIN32)
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86")
    set(GLEW_PROCESSOR_ARCH_PATH "Win32")
  elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
    set(GLEW_PROCESSOR_ARCH_PATH "Win32") #"x64") !!!! this seems to fix liking errors on MSVC that may be incorrectly configured for 64bit build
  endif() #why doesn't this work?
  message("glew processor arch path: " "${GLEW_LOCATION}/lib/Release/${GLEW_PROCESSOR_ARCH_PATH}")

  if(MSVC)
    find_path(GLEW_INCLUDE_DIRS
      NAMES GL/glew.h
      PATHS
      "${GLEW_LOCATION}/include"
      "$ENV{GLEW_LOCATION}/include"
      "${PROJECT_SOURCE_DIR}/libraries/glew/include")
    find_library(GLEW_LIBRARIES NAMES glew32$<$<CONFIG:Debug>:d>.lib
      PATHS
      "${GLEW_LOCATION}/lib/$<CONFIG>/${GLEW_PROCESSOR_ARCH_PATH}"
      "$ENV{GLEW_LOCATION}/lib/$<CONFIG>/${GLEW_PROCESSOR_ARCH_PATH}"
      "${PROJECT_SOURCE_DIR}/libraries/glew/$<CONFIG>/${GLEW_PROCESSOR_ARCH_PATH}") #no option for static vs dynamic link
    #find_library(GLEW_DEBUG_LIBRARIES NAMES glew32d.lib
    #  PATHS
    #  "${GLEW_LOCATION}/lib/Debug/${GLEW_PROCESSOR_ARCH_PATH}"
    #  "$ENV{GLEW_LOCATION}/lib/Debug/${GLEW_PROCESSOR_ARCH_PATH}"
    #  "${PROJECT_SOURCE_DIR}/libraries/glew/Debug/${GLEW_PROCESSOR_ARCH_PATH}")
    #if(NOT DEFINED GLEW_DEBUG_LIBRARIES)
    #  message("No GLEW debug binaries!")
    #  set(GLEW_DEBUG_LIBRARIES ${GLEW_LIBRARIES})
    #endif()
  elseif(CYGWIN)
    message(FATAL_ERROR "Cygwin not yet supported!")
  endif()
elseif(UNIX)
  find_package(PkgConfig)
  pkg_search_module(GLEW REQUIRED glew32)
elseif(APPLE)
  find_package(PkgConfig)
  if(PKG_CONFIG_FOUND)
    pkg_search_module(GLEW REQUIRED glew32)
  else()
    message(FATAL_ERROR "Mac OS X builds without pkg-config not yet supported")
  endif()
endif()

#hax!
set(GLEW_LIBRARIES ${GLEW_LOCATION}/lib/$<CONFIG>/${GLEW_PROCESSOR_ARCH_PATH}/glew32$<$<CONFIG:Debug>:d>.lib)
message("Path found for GLEW include dir: " ${GLEW_INCLUDE_DIRS})
message("Path found for GLEW library: " ${GLEW_LIBRARIES})
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLEW DEFAULT_MSG
  GLEW_INCLUDE_DIRS
  GLEW_LIBRARIES)
