project(Tractography)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

 
# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
endif(NOT Slicer3_SOURCE_DIR)

# --------------------------------------------------------------------------

# Warning - Do not include Logic subdirectory - This has been done in Slicer3/CMakeLists.txt

SET(Tractography_libs
  Seeding
  Settings
  Display
  FiducialSeeding
  Editor
  )
FOREACH(lib ${Tractography_libs})
  ADD_SUBDIRECTORY(${lib})
ENDFOREACH(lib)

# --------------------------------------------------------------------------
if(Slicer3_BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(Slicer3_BUILD_TESTING)

# Install testing data

file(GLOB_RECURSE MRMLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.mrml")
file(GLOB_RECURSE TCLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.tcl")
file(GLOB_RECURSE VTKFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.vtk")

foreach(file 
    ${MRMLFILES}
    ${TCLFILES}
    ${VTKFILES}
    )
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/${file}
    COPYONLY
    )
endforeach(file) 

install(DIRECTORY
  Testing
  DESTINATION ${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME} COMPONENT Development
  PATTERN ".svn" EXCLUDE
  PATTERN "CMakeLists.txt" EXCLUDE
  )
