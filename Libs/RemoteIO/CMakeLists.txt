PROJECT(RemoteIO)

# --------------------------------------------------------------------------
# Special build configs (to handle linking on windows)
CONFIGURE_FILE(
  ${RemoteIO_SOURCE_DIR}/vtkRemoteIOConfigure.h.in 
  ${RemoteIO_BINARY_DIR}/vtkRemoteIOConfigure.h
)


# --------------------------------------------------------------------------
# Settings for options
SET(RemoteIO_SRCS
        vtkDataTransfer.cxx
        vtkCacheManager.cxx
        vtkDataIOManager.cxx
        vtkPermissionPrompter.cxx
)


# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${VTK_INCLUDE_DIRS}
  ${RemoteIO_SOURCE_DIR}
  ${RemoteIO_BINARY_DIR}
  )


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
  )


# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
#  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

