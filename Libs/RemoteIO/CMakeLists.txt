PROJECT(RemoteIO)

# --------------------------------------------------------------------------
# Special build configs (to handle linking on windows)
CONFIGURE_FILE(
  ${RemoteIO_SOURCE_DIR}/vtkRemoteIOConfigure.h.in 
  ${RemoteIO_BINARY_DIR}/vtkRemoteIOConfigure.h
)


#
# Disable MSVC 8 warnings
IF(WIN32)
  OPTION(DISABLE_MSVC8_DEPRECATED_WARNINGS 
         "Disable Visual Studio 8 deprecated warnings" ON)
  MARK_AS_ADVANCED(FORCE DISABLE_MSVC8_DEPRECATED_WARNINGS)  
  IF(DISABLE_MSVC8_DEPRECATED_WARNINGS)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
  ENDIF(DISABLE_MSVC8_DEPRECATED_WARNINGS)
ENDIF(WIN32)



# --------------------------------------------------------------------------
# Settings for options
SET(RemoteIO_SRCS
#        vtkHTTPHandler.cxx
)


# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${VTK_INCLUDE_DIRS}
  ${RemoteIO_SOURCE_DIR}
  ${RemoteIO_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  )


# --------------------------------------------------------------------------
# Find SLICERLIBCURL (slicercurl)
FIND_PACKAGE(SLICERLIBCURL REQUIRED)
INCLUDE(${SLICERLIBCURL_USE_FILE})


# --------------------------------------------------------------------------
# Wrapping
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(RemoteIO RemoteIO_TCL_SRCS "${RemoteIO_SRCS}" "")


# Build the library
ADD_LIBRARY(RemoteIO
  ${RemoteIO_SRCS} ${RemoteIO_TCL_SRCS}
  )
IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS RemoteIO
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib/Slicer3 COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(RemoteIO
  vtkIO
  vtkIOTCL
  vtkCommonTCL
  ${SLICERLIBCURL_LIBRARIES}
  MRML
  )


# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
#  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

