project(FileWatcher)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------


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

# Apply user-defined properties to the library target.
IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer_LIBRARY_PROPERTIES)

install(TARGETS ${lib_name}
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )


# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)
