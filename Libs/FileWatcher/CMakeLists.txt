project(FileWatcher)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif(NOT DEFINED BUILD_SHARED_LIBS)
 
# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_BINARY_DIR} 
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/FileWatcherConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/FileWatcherConfigure.h
  )

# --------------------------------------------------------------------------
# Sources

set(FileWatcher_SRCS
  FileWatcher.cpp
  FileWatcherLinux.cpp
  FileWatcherWin32.cpp
  FileWatcherOSX.cpp
  )

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name FileWatcher)
  add_library(${lib_name}
  ${FileWatcher_SRCS}
)

set(link_libs
  )
if(NOT WIN32 AND NOT APPLE AND NOT UNIX)
  set(link_libs ${link_libs})
endif(NOT WIN32 AND NOT APPLE AND NOT UNIX)
if(UNIX)
  set(link_libs ${link_libs})
endif(UNIX)

target_link_libraries(${lib_name} ${link_libs})

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)
