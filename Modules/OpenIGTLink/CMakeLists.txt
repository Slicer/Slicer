PROJECT(OpenIGTLink)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

INCLUDE(${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer3_SOURCE_DIR)


# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/igtlutil
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  )

#
# Configure include file
#
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkOpenIGTLinkConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkOpenIGTLinkConfigure.h
  )

file(GLOB headers 
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/igtlutil/*.h"
  )
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkOpenIGTLinkConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )


# --------------------------------------------------------------------------
# Type Size Check for igtlutil library

INCLUDE (CheckTypeSize)
CHECK_TYPE_SIZE(int         CMAKE_SIZEOF_INT)
CHECK_TYPE_SIZE(long        CMAKE_SIZEOF_LONG)
CHECK_TYPE_SIZE("void*"     CMAKE_SIZEOF_VOID_P)
CHECK_TYPE_SIZE(char        CMAKE_SIZEOF_CHAR)
CHECK_TYPE_SIZE(short       CMAKE_SIZEOF_SHORT)
CHECK_TYPE_SIZE(float       CMAKE_SIZEOF_FLOAT)
CHECK_TYPE_SIZE(double      CMAKE_SIZEOF_DOUBLE)
CHECK_TYPE_SIZE("long long" CMAKE_SIZEOF_LONG_LONG)
CHECK_TYPE_SIZE("__int64"   CMAKE_SIZEOF___INT64)
CHECK_TYPE_SIZE("int64_t"   CMAKE_SIZEOF_INT64_T)

ADD_DEFINITIONS(-DIGTL_SIZEOF_CHAR=${CMAKE_SIZEOF_CHAR})
ADD_DEFINITIONS(-DIGTL_SIZEOF_DOUBLE=${CMAKE_SIZEOF_DOUBLE})
ADD_DEFINITIONS(-DIGTL_SIZEOF_FLOAT=${CMAKE_SIZEOF_FLOAT})
ADD_DEFINITIONS(-DIGTL_SIZEOF_INT=${CMAKE_SIZEOF_INT})
ADD_DEFINITIONS(-DIGTL_SIZEOF_LONG=${CMAKE_SIZEOF_LONG})
ADD_DEFINITIONS(-DIGTL_SIZEOF_SHORT=${CMAKE_SIZEOF_SHORT})
ADD_DEFINITIONS(-DIGTL_SIZEOF_FLOAT=${CMAKE_SIZEOF_FLOAT})
ADD_DEFINITIONS(-DIGTL_SIZEOF_DOUBLE=${CMAKE_SIZEOF_DOUBLE})

IF(CMAKE_SIZEOF_LONG_LONG)
  ADD_DEFINITIONS(-DIGTL_TYPE_USE_LONG_LONG=1)
  ADD_DEFINITIONS(-DIGTL_SIZEOF_LONG_LONG=${CMAKE_SIZEOF_LONG_LONG})
ELSE(CMAKE_SIZEOF_LONG_LONG)
IF(CMAKE_SIZEOF___INT64)
  ADD_DEFINITIONS(-DIGTL_TYPE_USE___INT64=1)
  ADD_DEFINITIONS(-DIGTL_SIZEOF___INT64=${CMAKE_SIZEOF___INT64})
ELSE(CMAKE_SIZEOF___INT64)
IF(CMAKE_SIZEOF_INT64_T)
  ADD_DEFINITIONS(-DIGTL_TYPE_USE_INT64_T=1)
  ADD_DEFINITIONS(-DIGTL_SIZEOF_INT64_T=${CMAKE_SIZEOF_INT64_T})
ENDIF(CMAKE_SIZEOF_INT64_T)
ENDIF(CMAKE_SIZEOF___INT64)
ENDIF(CMAKE_SIZEOF_LONG_LONG)

# --------------------------------------------------------------------------
# Sources

set(OpenIGTLink_SRCS 
  vtkOpenIGTLinkLogic.cxx
  vtkOpenIGTLinkGUI.cxx
  vtkIGTLConnector.cxx
  vtkIGTLCircularBuffer.cxx
)

set(OpenIGTLinkUtil_SRCS
  igtlutil/igtl_header.c
  igtlutil/igtl_image.c
  igtlutil/igtl_transform.c
  igtlutil/igtl_util.c
)


# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(OpenIGTLink OpenIGTLink_TCL_SRCS "${OpenIGTLink_SRCS}" "")



#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(OpenIGTLink_SRCS OpenIGTLink.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name OpenIGTLink)
add_library(${lib_name}
  ${OpenIGTLink_SRCS} 
  ${OpenIGTLink_TCL_SRCS}
  ${OpenIGTLinkUtil_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  )

slicer3_install_modules(${lib_name})

 
