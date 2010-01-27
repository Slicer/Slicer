PROJECT(MRMLCLI)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

# --------------------------------------------------------------------------
# Include dirs

SET(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

SET(packages
  MRML
  ModuleDescriptionParser
  )
  
FOREACH(package 
    ${packages}
    )
  # This is mostly to test building this lib as standalone outside Slicer3
  IF(NOT DEFINED ${package}_SOURCE_DIR)
    SET(${package}_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../${package}")
  ENDIF(NOT DEFINED ${package}_SOURCE_DIR)
  IF(NOT DEFINED ${package}_BINARY_DIR)
    STRING(REPLACE ${PROJECT_NAME} ${package}
      ${package}_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    LINK_DIRECTORIES(${${package}_BINARY_DIR})
  ENDIF(NOT DEFINED ${package}_BINARY_DIR)

  SET(include_dirs ${include_dirs}
    ${${package}_SOURCE_DIR}
    ${${package}_BINARY_DIR}
    )
ENDFOREACH(package)

INCLUDE_DIRECTORIES(${include_dirs})

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkMRMLCLIConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkMRMLCLIConfigure.h
  )

FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
INSTALL(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkMRMLCLIConfigure.h"
  DESTINATION include/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

SET(MRMLCLI_SRCS
  vtkMRMLCommandLineModuleNode.cxx
  )

# --------------------------------------------------------------------------
# Sources

# Wrapping
if (VTK_WRAP_TCL)
   include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake") 
   vtk_wrap_tcl3(MRMLCLI  MRMLCLI_TCL_SRCS   "${MRMLCLI_SRCS}" "")
endif (VTK_WRAP_TCL)

SET(srcs
 ${MRMLCLI_SRCS}
 )

if (VTK_WRAP_TCL)
  set(srcs
        ${srcs}
        ${MRMLCLI_TCL_SRCS}
   )
endif (VTK_WRAP_TCL)

# Build and install the library 
SET(lib_name MRMLCLI)
ADD_LIBRARY(${lib_name} 
  ${srcs}
 )

SET(libs
  MRML
  ModuleDescriptionParser
  )

TARGET_LINK_LIBRARIES(${lib_name}
  ${libs}
  )

# Apply user-defined properties to the library target.
IF(Slicer3_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer3_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer3_LIBRARY_PROPERTIES)

INSTALL(TARGETS ${lib_name}
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )
 
